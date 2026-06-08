#include <struct-jit/struct-jit.h>
#include "transfer.h"
#include <stdexcept>
#include <cstring>
#include <cmath>
#include <utility>

#if defined(_MSC_VER)
#  define SJIT_UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#  define SJIT_UNREACHABLE() __builtin_unreachable()
#else
#  define SJIT_UNREACHABLE() raise("struct_jit: unreachable code reached!")
#endif

#if defined(__clang__)
// This warning flag is Clang-specific; GCC does not recognize it (and does not
// warn here under -Wall -Wextra), so guard on __clang__ rather than __GNUC__.
#  pragma GCC diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

#include "half.h"
#include "srgb.h"
#include "dither.h"
#if defined(_MSC_VER)
// C4324: robin_map pads buckets to honor an alignment specifier (benign).
#  pragma warning(push)
#  pragma warning(disable: 4324)
#endif
#include <tsl/robin_map.h>
#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
#include <functional>
#include <memory>
#include <mutex>

NAMESPACE_BEGIN(struct_jit)

/// Mutable scratch register holding a single scalar value as it is loaded,
/// numerically converted, and stored by the software fallback path.
struct Temp {
    Type type = Type::Invalid;
    uint64_t value = 0;
};

/// Reverse the byte order of a `size`-byte value in place (endianness swap).
static void bswap(uint8_t *data, size_t size) {
    // `size` is always the width of a scalar field (<= 8). Telling the compiler
    // this keeps GCC's -O3 loop analysis from mis-bounding the swap and emitting
    // a spurious -Wstringop-overflow on the small stack buffers passed in here.
    if (size > 8)
        SJIT_UNREACHABLE();
    for (size_t i = 0; i < size / 2; ++i) {
        uint8_t t = data[i];
        data[i] = data[size - 1 - i];
        data[size - 1 - i] = t;
    }
}

// ----------------------------------------------------------------------------
// Software fallback: a portable scalar converter used when no JIT backend is
// available (or when the user requests `jit=false`). It mirrors the semantics
// of the generated kernel so both paths produce identical results.
// ----------------------------------------------------------------------------

template <typename Target, typename Source>
static void convert_scalar_3(void *ptr, bool norm, double dither) {
    Source source;
    memcpy(&source, ptr, sizeof(Source));

    // Float -> int: scale into the integer domain, add dither, round to nearest
    // (ties-to-even), then saturate to the destination range, all in the source
    // (= working) precision so the result matches the JIT bit-for-bit, including
    // at single precision. (half is never the working type; promote it to float
    // so this dead instantiation compiles.)
    if constexpr (!std::is_integral_v<Source> && std::is_integral_v<Target>) {
        using F = std::conditional_t<std::is_same_v<Source, half>, float, Source>;
        F v = (F) source;
        if (norm)
            v *= (F) std::numeric_limits<Target>::max();
        v = std::rint(v + (F) dither);

        // Clamp before the (truncating) cast, mirroring the JIT's min_f/max_f.
        // Both types are known here, so the bounds fold to compile-time constants.
        constexpr std::pair<double, double> b = int_clamp_bounds<Target, F>();
        if (v < (F) b.first)  v = (F) b.first;
        if (v > (F) b.second) v = (F) b.second;
        source = (Source) v;
    }

    Target target = (Target) source;

    // Int -> float: normalize by multiplying with the reciprocal of the source
    // range, in the target (= working) precision, as the JIT's scale snippet does.
    if constexpr (std::is_integral_v<Source> && !std::is_integral_v<Target>) {
        if (norm) {
            using F = std::conditional_t<std::is_same_v<Target, half>, float, Target>;
            target = (Target) ((F) target * (F) (1.0 / (double) std::numeric_limits<Source>::max()));
        }
    }

    memcpy(ptr, &target, sizeof(Target));
}

template <typename Target> static void convert_scalar_2(Temp &t, bool norm, double dither) {
    void *d = &t.value;
    switch (t.type) {
        case Type::Int8:    convert_scalar_3<Target, int8_t>   (d, norm, dither); break;
        case Type::UInt8:   convert_scalar_3<Target, uint8_t>  (d, norm, dither); break;
        case Type::Int16:   convert_scalar_3<Target, int16_t>  (d, norm, dither); break;
        case Type::UInt16:  convert_scalar_3<Target, uint16_t> (d, norm, dither); break;
        case Type::Int32:   convert_scalar_3<Target, int32_t>  (d, norm, dither); break;
        case Type::UInt32:  convert_scalar_3<Target, uint32_t> (d, norm, dither); break;
        case Type::Int64:   convert_scalar_3<Target, int64_t>  (d, norm, dither); break;
        case Type::UInt64:  convert_scalar_3<Target, uint64_t> (d, norm, dither); break;
        case Type::Float16: convert_scalar_3<Target, half>     (d, norm, dither); break;
        case Type::Float32: convert_scalar_3<Target, float>    (d, norm, dither); break;
        case Type::Float64: convert_scalar_3<Target, double>   (d, norm, dither); break;
        default:
            raise("struct_jit::convert_scalar(): invalid Target type!");
    }
}

static void convert_scalar(Temp &t, Type target, bool norm, double dither = 0.0) {
    switch (target) {
        case Type::Int8:    convert_scalar_2<int8_t>   (t, norm, dither); break;
        case Type::UInt8:   convert_scalar_2<uint8_t>  (t, norm, dither); break;
        case Type::Int16:   convert_scalar_2<int16_t>  (t, norm, dither); break;
        case Type::UInt16:  convert_scalar_2<uint16_t> (t, norm, dither); break;
        case Type::Int32:   convert_scalar_2<int32_t>  (t, norm, dither); break;
        case Type::UInt32:  convert_scalar_2<uint32_t> (t, norm, dither); break;
        case Type::Int64:   convert_scalar_2<int64_t>  (t, norm, dither); break;
        case Type::UInt64:  convert_scalar_2<uint64_t> (t, norm, dither); break;
        case Type::Float16: convert_scalar_2<half>     (t, norm, dither); break;
        case Type::Float32: convert_scalar_2<float>    (t, norm, dither); break;
        case Type::Float64: convert_scalar_2<double>   (t, norm, dither); break;
        default:
            raise("struct_jit::convert_scalar(): invalid target type!");
    }
    t.type = target;
}

Converter::Converter(const Struct &source, const Struct &target, bool jit, bool dither,
                     Type working_precision)
    : m_source(source), m_target(target), m_dither(dither), m_working(working_precision),
      m_kernel(nullptr), m_kernel_size(0) {
    if (working_precision != Type::Float32 && working_precision != Type::Float64)
        raise("Converter(): working_precision must be Float32 or Float64!");
    create_plan();
    if (jit)
        create_kernel();
}

Converter::~Converter() {
    if (m_kernel)
        release_kernel();
}

Converter::Converter(Converter &&c) noexcept
    : m_source(std::move(c.m_source)),
      m_target(std::move(c.m_target)),
      m_plan(std::move(c.m_plan)),
      m_weight_divide(c.m_weight_divide),
      m_weight_in(c.m_weight_in),
      m_alpha_apply(c.m_alpha_apply),
      m_alpha_in(c.m_alpha_in),
      m_blend(std::move(c.m_blend)),
      m_dither(c.m_dither),
      m_working(c.m_working),
      m_kernel(c.m_kernel),
      m_kernel_size(c.m_kernel_size) {
    c.m_kernel = nullptr;
    c.m_kernel_size = 0;
}

Converter &Converter::operator=(Converter &&c) noexcept {
    if (this == &c)
        return *this;

    if (m_kernel)
        release_kernel();

    m_source = std::move(c.m_source);
    m_target = std::move(c.m_target);
    m_plan = std::move(c.m_plan);
    m_weight_divide = c.m_weight_divide;
    m_weight_in = c.m_weight_in;
    m_alpha_apply = c.m_alpha_apply;
    m_alpha_in = c.m_alpha_in;
    m_blend = std::move(c.m_blend);
    m_dither = c.m_dither;
    m_working = c.m_working;
    m_kernel = c.m_kernel;
    m_kernel_size = c.m_kernel_size;

    c.m_kernel = nullptr;
    c.m_kernel_size = 0;
    return *this;
}

static void hash_combine(size_t &seed, size_t value) {
    seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6) + (seed >> 2);
}

static size_t hash_field(const Field &field) {
    size_t result = 0;
    hash_combine(result, std::hash<std::string>()(field.name));
    hash_combine(result, std::hash<std::string>()(field.source));
    hash_combine(result, std::hash<uint32_t>()((uint32_t) field.type));
    hash_combine(result, std::hash<size_t>()(field.offset));
    hash_combine(result, std::hash<uint32_t>()(field.flags));
    hash_combine(result, std::hash<double>()(field.value));
    for (const auto &term : field.blend) {
        hash_combine(result, std::hash<double>()(term.first));
        hash_combine(result, std::hash<std::string>()(term.second));
    }
    return result;
}

static size_t hash_struct(const Struct &s) {
    size_t result = 0;
    hash_combine(result, std::hash<bool>()(s.pack()));
    hash_combine(result, std::hash<uint32_t>()((uint32_t) s.byte_order()));
    hash_combine(result, std::hash<size_t>()(s.size()));
    for (const Field &field : s)
        hash_combine(result, hash_field(field));
    return result;
}

struct ConverterCacheKey {
    Struct source;
    Struct target;
    bool jit;
    bool dither;
    Type working_precision;

    bool operator==(const ConverterCacheKey &other) const {
        return source == other.source &&
               target == other.target &&
               jit == other.jit &&
               dither == other.dither &&
               working_precision == other.working_precision;
    }
};

struct ConverterCacheKeyHash {
    size_t operator()(const ConverterCacheKey &key) const {
        size_t result = 0;
        hash_combine(result, hash_struct(key.source));
        hash_combine(result, hash_struct(key.target));
        hash_combine(result, std::hash<bool>()(key.jit));
        hash_combine(result, std::hash<bool>()(key.dither));
        hash_combine(result, std::hash<uint32_t>()((uint32_t) key.working_precision));
        return result;
    }
};

namespace {

class ConverterCache {
public:
    const Converter &get(const Struct &source, const Struct &target, bool jit,
                         bool dither, Type working_precision) {
        std::lock_guard<std::mutex> lock(m_mutex);

        ConverterCacheKey key { source, target, jit, dither, working_precision };
        auto it = m_entries.find(key);
        if (it != m_entries.end())
            return *it->second;

        std::unique_ptr<Converter> converter(
            new Converter(source, target, jit, dither, working_precision));
        const Converter &result = *converter;
        m_entries.emplace(std::move(key), std::move(converter));
        return result;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
    }

private:
    mutable std::mutex m_mutex;
    tsl::robin_map<ConverterCacheKey, std::unique_ptr<Converter>,
                   ConverterCacheKeyHash> m_entries;
};

static ConverterCache &shared_converter_cache() {
    static ConverterCache cache;
    return cache;
}

} // namespace

const Converter &make_converter(const Struct &source, const Struct &target, bool jit,
                                bool dither, Type working_precision) {
    return shared_converter_cache().get(source, target, jit, dither,
                                        working_precision);
}

void clear_cache() { shared_converter_cache().clear(); }


/// Locate the single field carrying \c flag in \c s, raising on duplicates when
/// \c unique is set. Returns \ref None if absent; sets \c multiple when more than
/// one match exists (used for the alpha "multiple channels" diagnostic).
static size_t find_flagged(const Struct &s, Flag flag, const char *what,
                           bool unique, bool *multiple = nullptr) {
    size_t result = None;
    for (size_t i = 0; i < s.size(); ++i) {
        if (!has_flag(s[i].flags, flag))
            continue;
        if (result != None) {
            if (unique)
                raise(std::string("Converter::create_plan(): the data structure "
                                  "contains multiple ") + what + " fields!");
            if (multiple)
                *multiple = true;
        } else {
            result = i;
        }
    }
    return result;
}

void Converter::create_plan() {
    m_source.validate();
    m_target.validate();

    // ---- Weight fields (at most one per structure) ----
    size_t weight_in  = find_flagged(m_source,  Flag::Weight, "weight", true),
           weight_out = find_flagged(m_target, Flag::Weight, "weight", true);
    if (weight_in != None && weight_out != None &&
        m_source[weight_in].name != m_target[weight_out].name)
        raise("Converter::create_plan(): the weight fields of the input (\"" +
              m_source[weight_in].name + "\") and output (\"" +
              m_target[weight_out].name + "\") data structure have mismatched names!");

    // Converting weighted -> unweighted (input weighted, output not) divides
    // every mapped field by the weight; the backend loads it up front.
    m_weight_divide = weight_in != None && weight_out == None;
    m_weight_in = m_weight_divide ? weight_in : None;

    // ---- Alpha fields ----
    bool alpha_in_multiple = false;
    size_t alpha_in  = find_flagged(m_source,  Flag::Alpha, "alpha", false, &alpha_in_multiple),
           alpha_out = find_flagged(m_target, Flag::Alpha, "alpha", false);
    if (alpha_in != None && alpha_out != None &&
        m_source[alpha_in].name != m_target[alpha_out].name)
        raise("Converter::create_plan(): the alpha fields of the input (\"" +
              m_source[alpha_in].name + "\") and output (\"" +
              m_target[alpha_out].name + "\") data structure have mismatched names!");
    m_alpha_apply = alpha_in != None && alpha_out != None;
    m_alpha_in = m_alpha_apply ? alpha_in : None;

    // ---- Per-output-field plan ----
    bool any_premult_conv = false;
    for (size_t i = 0; i < m_target.size(); ++i) {
        const Field &f = m_target[i];

        // Blended output: resolve each term's source field by name. The result
        // is a linear combination computed in the working precision (see the
        // backends); blend fields bypass the normal name lookup and alpha.
        if (!f.blend.empty()) {
            BlendEntry be;
            be.output = i;
            for (const std::pair<double, std::string> &term : f.blend) {
                Struct::ConstFieldIterator it = m_source.find(term.second);
                if (it == m_source.end())
                    raise("Converter::create_plan(): the blend source field \"" +
                          term.second + "\" (for output \"" + f.name +
                          "\") could not be found in the input.");
                be.terms.emplace_back((size_t) (it - m_source.begin()), term.first);
            }
            m_blend.push_back(std::move(be));
            continue;
        }

        // A non-empty `source` redirects the lookup to a differently-named input
        // field, expressing a plain renamed copy (the output keeps `name`).
        const std::string &src_name = f.source.empty() ? f.name : f.source;
        Struct::ConstFieldIterator it = m_source.find(src_name);
        std::pair<size_t, size_t> entry;
        if (it != m_source.end())
            entry = { (size_t) (it - m_source.begin()), i };
        else if (f.source.empty() && has_flag(f.flags, Flag::Default))
            entry = { None, i };
        else
            raise("Converter::create_plan(): the output data structure "
                  "contains a field with name \"" + f.name + "\" (reading from "
                  "input field \"" + src_name + "\") that could not be found in "
                  "the input, and which lacks a default initialization.");

        m_plan.push_back(entry);

        // Only needed to police the multiple-alpha rule below, so skip the work
        // unless there actually is more than one alpha channel to disambiguate.
        if (m_alpha_apply && alpha_in_multiple) {
            Transfer t = make_transfer(m_source, m_target, entry, m_working,
                                       m_weight_divide, true);
            any_premult_conv |= t.alpha_premul || t.alpha_unpremul;
        }
    }

    // Only reject multiple alpha channels when a premultiplication conversion
    // would actually consume one (a single alpha is then ambiguous).
    if (m_alpha_apply && alpha_in_multiple && any_premult_conv)
        raise("Converter::create_plan(): multiple alpha channels found; alpha "
              "(un)premultiplication requires a single alpha channel!");
}

bool Converter::convert(const void *in, void *out, size_t width,
                        size_t height) const {
    // Nothing to do for an empty region. This also shields the JIT kernel,
    // whose loops test the counter after the body (do-while), from running a
    // spurious iteration and then wrapping around for a zero extent.
    if (width == 0 || height == 0)
        return true;

    if (m_kernel)
        return m_kernel(in, out, width, height);
    else
        return convert_fallback((const uint8_t *) in, (uint8_t *) out, width, height);
}

bool Converter::convert_fallback(const uint8_t *in, uint8_t *out, size_t width, size_t height) const {
    return m_working == Type::Float32 ? convert_fallback_impl<float>(in, out, width, height)
                                      : convert_fallback_impl<double>(in, out, width, height);
}

template <typename Float>
bool Converter::convert_fallback_impl(const uint8_t *in_base, uint8_t *out_base,
                                      size_t width, size_t height) const {
    size_t in_size = m_source.nbytes(), out_size = m_target.nbytes();

    // Resolve every plan entry into its shared Transfer recipe once, up front:
    // the recipe is identical for every record, so there is no need to rebuild
    // it per pixel (the JIT likewise resolves it a single time, at compile time).
    std::vector<Transfer> transfers;
    transfers.reserve(m_plan.size());
    for (const std::pair<size_t, size_t> &entry : m_plan)
        transfers.push_back(make_transfer(m_source, m_target, entry, m_working,
                                           m_weight_divide, m_alpha_apply));

    Temp temp;

    // Record cursor and pixel coordinates shared with the lambdas below; updated
    // as the loop walks the (row-major, contiguous) input and output buffers.
    const uint8_t *in = in_base;
    uint8_t *out = out_base;
    size_t x = 0, y = 0;

    // The value is held in the working float precision \c Float between the
    // input and output conversions; read/write it directly (no type dispatch).
    auto read  = [&] { Float v; memcpy(&v, &temp.value, sizeof(Float)); return v; };
    auto write = [&](Float v) { memcpy(&temp.value, &v, sizeof(Float)); };

    // Load a source field and linearize it to the working precision: read,
    // byte-swap, int->float (with normalization), float precision adjust, and
    // sRGB decode. Shared by the weight/alpha preamble and the blend terms.
    auto load_linearized = [&](const Field &fi) -> Float {
        Temp tmp;
        memcpy(&tmp.value, in + fi.offset, type_size(fi.type));
        tmp.type = fi.type;
        if (m_source.byte_order() != native_byte_order())
            bswap((uint8_t *) &tmp.value, type_size(tmp.type));
        convert_scalar(tmp, m_working, has_flag(fi.flags, Flag::Normalized));
        Float v;
        memcpy(&v, &tmp.value, sizeof(Float));
        if (has_flag(fi.flags, Flag::Gamma))
            v = srgb_to_linear(v);
        return v;
    };

    // Store the current \c temp into an output field: sRGB encode (if the value
    // is a linear working float), convert (with dither on float->int), byte-swap,
    // write. Shared by the value-field tail and the blend outputs; the JIT mirror
    // is emit_store_working().
    auto finish_output = [&](const Field &fo, bool gamma_encode) {
        if (gamma_encode)
            write(linear_to_srgb(read()));
        if (fo.type != temp.type) {
            double dither = 0.0;
            if (m_dither && type_is_float(temp.type) && !type_is_float(fo.type))
                dither = dither_matrix256[(y % 256) * 256 + (x % 256)];
            convert_scalar(temp, fo.type, has_flag(fo.flags, Flag::Normalized), dither);
        }
        if (m_target.byte_order() != native_byte_order())
            bswap((uint8_t *) &temp.value, type_size(fo.type));
        memcpy(out + fo.offset, &temp.value, type_size(fo.type));
    };

    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            // Up front (like the JIT): verify every Check-flagged input field
            // against its expected raw value, independent of the conversion
            // plan, so input-only check fields are covered. A mismatch fails the
            // whole conversion.
            for (size_t i = 0; i < m_source.size(); ++i) {
                const Field &fi = m_source[i];
                if (!has_flag(fi.flags, Flag::Check))
                    continue;
                uint64_t raw = 0;
                memcpy(&raw, in + fi.offset, type_size(fi.type));
                if (m_source.byte_order() != native_byte_order())
                    bswap((uint8_t *) &raw, type_size(fi.type));
                uint64_t expected = encode_value(fi);
                if (memcmp(&raw, &expected, type_size(fi.type)) != 0)
                    return false;
            }

            // Preamble (per record, before any field is written): the weight
            // reciprocal (1 if zero; NaN propagates) and the alpha / inverse alpha.
            Float weight_recip = 1, alpha = 1, inv_alpha = 1;
            if (m_weight_divide) {
                Float w = load_linearized(m_source[m_weight_in]);
                weight_recip = w == Float(0) ? Float(1) : Float(1) / w;
            }
            if (m_alpha_apply) {
                alpha = load_linearized(m_source[m_alpha_in]);
                inv_alpha = alpha == Float(0) ? Float(0) : Float(1) / alpha;
            }

            for (const Transfer &t : transfers) {
                if (t.input) {
                    const Field &fi = *t.input;
                    memcpy(&temp.value, in + fi.offset, type_size(fi.type));
                    temp.type = fi.type;

                    if (m_source.byte_order() != native_byte_order())
                        bswap((uint8_t *) &temp.value, type_size(temp.type));

                    if (t.needs_conversion)
                        convert_scalar(temp, t.working_type, t.input_normalized);

                    if (t.gamma_decode)
                        write(srgb_to_linear(read()));
                } else {
                    // Input field is missing; substitute the default value.
                    const Field &fo = *t.output;
                    uint64_t def = encode_value(fo);
                    memcpy(&temp.value, &def, type_size(fo.type));
                    temp.type = fo.type;
                }

                // Weight division, then alpha (un)premultiplication.
                if (t.weight_apply)
                    write(read() * weight_recip);
                if (t.alpha_premul)
                    write(read() * alpha);
                if (t.alpha_unpremul)
                    write(read() * inv_alpha);

                finish_output(*t.output, t.gamma_encode);
            }

            // Blended outputs: sum the weighted, linearized source terms in the
            // working precision, divide by the weight if requested, then store.
            // Blend fields do not participate in alpha (un)premultiplication.
            for (const BlendEntry &be : m_blend) {
                Float accum = 0;
                for (const std::pair<size_t, double> &term : be.terms)
                    accum += (Float) term.second * load_linearized(m_source[term.first]);
                if (m_weight_divide)
                    accum *= weight_recip;
                const Field &fo = m_target[be.output];
                temp.type = m_working;
                write(accum);
                finish_output(fo, has_flag(fo.flags, Flag::Gamma));
            }

            in += in_size;
            out += out_size;
        } // x
    } // y
    return true;
}


NAMESPACE_END(struct_jit)

#undef SJIT_UNREACHABLE
