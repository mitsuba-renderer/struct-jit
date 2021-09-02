#include <struct-jit/struct-jit.h>
#include <stdexcept>
#include <cstring>

NAMESPACE_BEGIN(struct_jit)

/// Convenience wrapper to raise an exception
static void raise(const std::string &msg) { throw std::runtime_error(msg); }

/// Indicates a missing source/destination field
constexpr size_t None = size_t(-1);

using Plan = std::vector<std::pair<size_t, size_t>>;

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

/**
 * Analyze the input data and output data structure and prepare a list of
 * fields that should be visited.
 */
static Plan create_plan(const Struct &in_struct, const Struct &out_struct) {
    Plan plan;
    size_t weight_in = None, weight_out = None;

    verify_flags(in_struct);
    verify_flags(out_struct);

    // Check whether the input data structure has a (single) weight field.
    for (size_t i = 0; i < in_struct.fields(); ++i) {
        const Field &f = in_struct[i];

        if (has_flag(f.flags, Flag::Weight)) {
            if (weight_in != None)
                raise("struct_jit::create_plan(): the input data structure "
                      "contains multiple weight fields!");
            weight_in = i;
        }
    }

    for (size_t i = 0; i < out_struct.fields(); ++i) {
        const Field &f = out_struct[i];
        bool is_weight = has_flag(f.flags, Flag::Weight);

        // Check whether the output data structure has a (single) weight field.
        if (is_weight) {
            if (weight_out != None)
                raise("struct_jit::create_plan(): the output data structure "
                      "contains multiple weight fields!");
            weight_out = i;
        }

        Struct::ConstFieldIterator it = in_struct.find(f.name);

        if (it != in_struct.end()) {
            if (!is_weight)
                plan.emplace_back(it - in_struct.begin(), i);
        } else if (has_flag(f.flags, Flag::Default)) {
            if (!is_weight)
                plan.emplace_back(None, i);
        } else {
            raise("struct_jit::create_plan(): the output data structure "
                  "contains a field with name \"" + f.name +
                  "\" that could not be found in the input, and which lacks a "
                  "default initialization.");
        }
    }

    if (weight_in != None || weight_out != None) {
        if (weight_in != None && weight_out != None) {
            const Field &fi = in_struct[weight_in],
                        &fo = out_struct[weight_out];

            if (fi.name != fo.name)
                raise("struct_jit::create_plan(): the weight fields of the input (\"" +
                      fi.name + "\") and output (\"" + fo.name +
                      "\") data structure have mismatched names!");
        }

        plan.insert(plan.begin(), std::make_pair(weight_in, weight_out));
    }

    return plan;
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
static void convert_3(void *ptr, bool norm) {
    Source source;
    memcpy(&source, ptr, sizeof(Source));

    if (!std::is_integral_v<Source> && std::is_integral_v<Target> && norm)
        source *= 1.f / Source(std::numeric_limits<Target>::max());

    Target target = (Target) source;

    if (std::is_integral_v<Source> && !std::is_integral_v<Target> && norm)
        target *= Target(std::numeric_limits<Source>::max());

    memcpy(ptr, &target, sizeof(Target));
}

template <typename Target> static void convert_2(Temp &t, bool norm) {
    void *d = &t.value;
    switch (t.type) {
        case Type::Int8:    convert_3<Target, int8_t>   (d, norm); break;
        case Type::UInt8:   convert_3<Target, uint8_t>  (d, norm); break;
        case Type::Int16:   convert_3<Target, int16_t>  (d, norm); break;
        case Type::UInt16:  convert_3<Target, uint16_t> (d, norm); break;
        case Type::Int32:   convert_3<Target, int32_t>  (d, norm); break;
        case Type::UInt32:  convert_3<Target, uint32_t> (d, norm); break;
        // case Type::Float16: convert_3<Target, half>     (d, norm); break;
        case Type::Float32: convert_3<Target, float>    (d, norm); break;
        case Type::Float64: convert_3<Target, double>   (d, norm); break;
        default:
            raise("struct_jit::convert(): invalid Target type!");
    }
}

static void convert(Temp &t, Type target, bool norm) {
    switch (target) {
        case Type::Int8:    convert_2<int8_t>   (t, norm); break;
        case Type::UInt8:   convert_2<uint8_t>  (t, norm); break;
        case Type::Int16:   convert_2<int16_t>  (t, norm); break;
        case Type::UInt16:  convert_2<uint16_t> (t, norm); break;
        case Type::Int32:   convert_2<int32_t>  (t, norm); break;
        case Type::UInt32:  convert_2<uint32_t> (t, norm); break;
        // case Type::Float16: convert_2<half>     (t, norm); break;
        case Type::Float32: convert_2<float>    (t, norm); break;
        case Type::Float64: convert_2<double>   (t, norm); break;
        default:
            raise("struct_jit::convert(): invalid target type!");
    }
    t.type = target;
}

bool transform_fallback(const Plan &plan, const uint8_t *in,
                        const Struct &in_struct, uint8_t *out,
                        const Struct &out_struct, size_t x, size_t y) {
    (void) x; (void) y;

    Temp temp;
    for (auto [i_in, i_out] : plan) {
        if (i_in != None) {
            const Field &f = in_struct[i_in];
            memcpy(&temp.value, in + f.offset, size(f.type));
            temp.type = f.type;
            temp.gamma = f.flags & (uint32_t) Flag::Gamma;

            if (in_struct.byte_order() != native_byte_order())
                bswap((uint8_t *) &temp.value, size(temp.type));

            // Ensure that the input value matches the specfied value
            if (f.flags & Flag::Check) {
                if (memcmp(&temp.value, &f.value, size(temp.type)) != 0)
                    return false;
            }

            bool normalize = f.flags & Flag::Normalized,
                 requires_conversion = normalize ||
                     (i_out != None && out_struct[i_out].type != temp.type);

            if (requires_conversion)
                convert(
                    temp, size(temp.type) >= 4 ? Type::Float64 : Type::Float32,
                    normalize);
        } else {
            // Input field is missing, substitute a default
            const Field &f = out_struct[i_out];
            memcpy(&temp.value, &f.value, size(f.type));
            temp.type = f.type;
        }

        if (i_out != None) {
            const Field &f = out_struct[i_out];

            if (f.type != temp.type)
                convert(temp, f.type, f.flags & Flag::Normalized);

            if (out_struct.byte_order() != native_byte_order())
                bswap((uint8_t *) &temp.value, size(f.type));

            memcpy(out + f.offset, &temp.value, size(f.type));
        }
    }
    return true;
}

bool transform(const void *in, const Struct &in_struct, void *out,
             const Struct &out_struct, size_t width, size_t height) {

    Plan plan = create_plan(in_struct, out_struct);

    const uint8_t *in_ptr = (const uint8_t *) in;
    uint8_t *out_ptr = (uint8_t *) out;
    size_t in_size = in_struct.size(),
           out_size = out_struct.size();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (!transform_fallback(plan, in_ptr, in_struct, out_ptr,
                                    out_struct, x, y))
                return false;

            in_ptr += in_size;
            out_ptr += out_size;
        }
    }

    return true;
}

NAMESPACE_END(struct_jit)
