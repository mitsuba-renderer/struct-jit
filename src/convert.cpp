#include <struct-jit/struct-jit.h>
#include <stdexcept>
#include <cstring>
#include <cmath>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

#include "half.h"

NAMESPACE_BEGIN(struct_jit)

/// Convenience wrapper to raise an exception
static void raise(const std::string &msg) { throw std::runtime_error(msg); }

/// Indicates a missing source/destination field
constexpr size_t None = size_t(-1);

static void verify_flags(const Struct &s) {
    for (size_t i = 0; i < s.fields(); ++i) {
        const Field &f = s[i];
        if (f.flags & Flag::Normalized) {
            if (is_float(f.type))
                raise("struct_jit::verify_flags(\"" + f.name +
                      "\"): 'Normalized' flag requires an integral field!");
        }

        if (f.flags & Flag::Gamma) {
            if (!(f.flags & Flag::Normalized))
                raise("struct_jit::verify_flags(\"" + f.name +
                      "\"): 'Gamma' flag requires that 'Normalized' is also "
                      "specified!");
        }

        if ((f.flags & Flag::Check) && (f.flags & Flag::Default)) {
            raise("struct_jit::verify_flags(\"" + f.name +
                  "\"): 'Check' and 'Default' flags cannot be specified at the "
                  "same time!");
        }
    }
}


struct Temp {
    Type type = Type::Invalid;
    uint64_t value = 0;
    bool gamma = false;
};

static void bswap(uint8_t *data, size_t size) {
    for (size_t i = 0; i < size / 2; ++i)
        std::swap(data[i], data[size - 1 - i]);
}

template <typename Target, typename Source>
static void convert_scalar_3(void *ptr, bool norm) {
    Source source;
    memcpy(&source, ptr, sizeof(Source));

    if constexpr (!std::is_integral_v<Source> && std::is_integral_v<Target>) {
        if (norm)
            source = Source(std::rint(double(source) * std::numeric_limits<Target>::max()));
    }

    Target target = (Target) source;

    if constexpr (std::is_integral_v<Source> && !std::is_integral_v<Target>) {
        if (norm)
            target = Target(double(target) / std::numeric_limits<Source>::max());
    }

    memcpy(ptr, &target, sizeof(Target));
}

template <typename Target> static void convert_scalar_2(Temp &t, bool norm) {
    void *d = &t.value;
    switch (t.type) {
        case Type::Int8:    convert_scalar_3<Target, int8_t>   (d, norm); break;
        case Type::UInt8:   convert_scalar_3<Target, uint8_t>  (d, norm); break;
        case Type::Int16:   convert_scalar_3<Target, int16_t>  (d, norm); break;
        case Type::UInt16:  convert_scalar_3<Target, uint16_t> (d, norm); break;
        case Type::Int32:   convert_scalar_3<Target, int32_t>  (d, norm); break;
        case Type::UInt32:  convert_scalar_3<Target, uint32_t> (d, norm); break;
        case Type::Int64:   convert_scalar_3<Target, int64_t>  (d, norm); break;
        case Type::UInt64:  convert_scalar_3<Target, uint64_t> (d, norm); break;
        case Type::Float16: convert_scalar_3<Target, half>     (d, norm); break;
        case Type::Float32: convert_scalar_3<Target, float>    (d, norm); break;
        case Type::Float64: convert_scalar_3<Target, double>   (d, norm); break;
        default:
            raise("struct_jit::convert_scalar(): invalid Target type!");
    }
}

static void convert_scalar(Temp &t, Type target, bool norm) {
    switch (target) {
        case Type::Int8:    convert_scalar_2<int8_t>   (t, norm); break;
        case Type::UInt8:   convert_scalar_2<uint8_t>  (t, norm); break;
        case Type::Int16:   convert_scalar_2<int16_t>  (t, norm); break;
        case Type::UInt16:  convert_scalar_2<uint16_t> (t, norm); break;
        case Type::Int32:   convert_scalar_2<int32_t>  (t, norm); break;
        case Type::UInt32:  convert_scalar_2<uint32_t> (t, norm); break;
        case Type::Int64:   convert_scalar_2<int64_t>  (t, norm); break;
        case Type::UInt64:  convert_scalar_2<uint64_t> (t, norm); break;
        case Type::Float16: convert_scalar_2<half>     (t, norm); break;
        case Type::Float32: convert_scalar_2<float>    (t, norm); break;
        case Type::Float64: convert_scalar_2<double>   (t, norm); break;
        default:
            raise("struct_jit::convert_scalar(): invalid target type!");
    }
    t.type = target;
}

Converter::Converter(const Struct &in, const Struct &out, bool jit)
    : m_in(in), m_out(out), m_jit(jit) {
    create_plan();
}

/**
 * Analyze the input data and output data structure and prepare a list of
 * fields that should be visited.
 */
void Converter::create_plan() {
    size_t weight_in = None, weight_out = None;

    verify_flags(m_in);
    verify_flags(m_out);

    // Check whether the input data structure has a (single) weight field.
    for (size_t i = 0; i < m_in.fields(); ++i) {
        const Field &f = m_in[i];

        if (has_flag(f.flags, Flag::Weight)) {
            if (weight_in != None)
                raise("Converter::create_plan(): the input data structure "
                      "contains multiple weight fields!");
            weight_in = i;
        }
    }

    for (size_t i = 0; i < m_out.fields(); ++i) {
        const Field &f = m_out[i];
        bool is_weight = has_flag(f.flags, Flag::Weight);

        // Check whether the output data structure has a (single) weight field.
        if (is_weight) {
            if (weight_out != None)
                raise("Converter::create_plan(): the output data structure "
                      "contains multiple weight fields!");
            weight_out = i;
        }

        Struct::ConstFieldIterator it = m_in.find(f.name);

        if (it != m_in.end()) {
            if (!is_weight)
                m_plan.emplace_back(it - m_in.begin(), i);
        } else if (has_flag(f.flags, Flag::Default)) {
            if (!is_weight)
                m_plan.emplace_back(None, i);
        } else {
            raise("Converter::create_plan(): the output data structure "
                  "contains a field with name \"" + f.name +
                  "\" that could not be found in the input, and which lacks a "
                  "default initialization.");
        }
    }

    if (weight_in != None || weight_out != None) {
        if (weight_in != None && weight_out != None) {
            const Field &fi = m_in[weight_in],
                        &fo = m_out[weight_out];

            if (fi.name != fo.name)
                raise("Converter::create_plan(): the weight fields of the input (\"" +
                      fi.name + "\") and output (\"" + fo.name +
                      "\") data structure have mismatched names!");
        }

        m_plan.insert(m_plan.begin(), std::make_pair(weight_in, weight_out));
    }
}

bool Converter::convert(const void *in, void *out, size_t width,
                        size_t height) const {
    const uint8_t *in_ptr = (const uint8_t *) in;
    uint8_t *out_ptr = (uint8_t *) out;
    size_t in_size = m_in.size(), out_size = m_out.size();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (!convert_slow(in_ptr, out_ptr, x, y))
                return false;

            in_ptr += in_size;
            out_ptr += out_size;
        }
    }

    return true;
}

bool Converter::convert_slow(const uint8_t *in, uint8_t *out, size_t x, size_t y) const {
    (void) x; (void) y;

    Temp temp;
    for (auto [i_in, i_out] : m_plan) {
        if (i_in != None) {
            const Field &f = m_in[i_in];
            memcpy(&temp.value, in + f.offset, size(f.type));
            temp.type = f.type;
            temp.gamma = f.flags & Flag::Gamma;

            if (m_in.byte_order() != native_byte_order())
                bswap((uint8_t *) &temp.value, size(temp.type));

            // Ensure that the input value matches the specfied value
            if (f.flags & Flag::Check) {
                if (memcmp(&temp.value, &f.value, size(temp.type)) != 0)
                    return false;
            }

            bool normalize = f.flags & Flag::Normalized,
                 requires_conversion = false;

            if (i_out != None) {
                const Field &fo = m_out[i_out];
                requires_conversion =
                    requires_conversion || fo.type != temp.type ||
                    (fo.flags & Flag::Gamma) != temp.gamma ||
                    fo.flags & Flag::Normalized;
            }

            if (requires_conversion)
                convert_scalar(
                    temp, size(temp.type) >= 4 ? Type::Float64 : Type::Float32,
                    normalize);
        } else {
            // Input field is missing, substitute a default
            const Field &f = m_out[i_out];
            memcpy(&temp.value, &f.value, size(f.type));
            temp.type = f.type;
        }

        if (i_out != None) {
            const Field &f = m_out[i_out];

            if (f.type != temp.type)
                convert_scalar(temp, f.type, f.flags & Flag::Normalized);

            if (m_out.byte_order() != native_byte_order())
                bswap((uint8_t *) &temp.value, size(f.type));

            memcpy(out + f.offset, &temp.value, size(f.type));
        }
    }
    return true;
}


NAMESPACE_END(struct_jit)
