/*
 * Internal (non-installed) header providing the single source of truth for the
 * static properties of each \ref Type: its name, byte size, representable
 * range, NumPy kind character, and integer/float classification. The public
 * type_* query functions, the converters, and the Python bindings all resolve
 * a Type to its \ref TypeInfo record through \ref type_info(). It also hosts a
 * couple of small shared helpers (\ref raise, \ref None, \ref copy_value,
 * \ref field_end) used across the implementation.
 */
#pragma once

#include <struct-jit/struct-jit.h>
#include "half.h"
#include "srgb.h"
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

NAMESPACE_BEGIN(struct_jit)

/// Throw a std::runtime_error carrying the given message (maps to RuntimeError
/// on the Python side). Marked [[noreturn]] so callers can treat it as a trap.
[[noreturn]] inline void raise(const std::string &msg) {
    throw std::runtime_error(msg);
}

/// Sentinel index meaning "no such field" in conversion plans and lookups.
constexpr size_t None = size_t(-1);

/// Shared layout limits supported by every backend. AArch64 is the tightest
/// target: record strides use an unsigned 12-bit ADD immediate, and unaligned
/// multi-byte fields use an unsigned byte offset in the unscaled load/store
/// snippets. Keep validation above the backend layer so JIT and fallback accept
/// exactly the same layouts.
constexpr size_t MaxRecordBytes = 4095;
constexpr size_t MaxUnalignedOffset = 255;

/// Static, compile-time description of one \ref Type.
struct TypeInfo {
    const char *name;       ///< Human-readable name, e.g. "uint8"
    size_t size;            ///< Size in bytes
    double min_value;       ///< Smallest representable value
    double max_value;       ///< Largest representable value
    char numpy_kind;        ///< NumPy dtype kind character ('i', 'u', 'f')
    bool signed_integer;    ///< Is this a signed integer type?
    bool unsigned_integer;  ///< Is this an unsigned integer type?
    bool floating_point;    ///< Is this a floating point type?
};

/// Build a \ref TypeInfo entry for an integer type, deriving size and range
/// from the C++ type \c T.
template <typename T>
constexpr TypeInfo make_int_type_info(const char *name, char numpy_kind,
                                      bool signed_integer, bool unsigned_integer) {
    return TypeInfo {
        name,
        sizeof(T),
        double(std::numeric_limits<T>::lowest()),
        double(std::numeric_limits<T>::max()),
        numpy_kind,
        signed_integer,
        unsigned_integer,
        false
    };
}

/// Build a \ref TypeInfo entry for a floating point type, deriving size and
/// range from the C++ type \c T.
template <typename T>
constexpr TypeInfo make_float_type_info(const char *name) {
    return TypeInfo {
        name,
        sizeof(T),
        double(std::numeric_limits<T>::lowest()),
        double(std::numeric_limits<T>::max()),
        'f',
        false,
        false,
        true
    };
}

// This table intentionally mirrors the public Type enum order so lookups stay
// branch-light and shared by the fallback, JIT, stream output, and bindings.
inline constexpr std::array<TypeInfo, 12> TypeInfos {{
    { "invalid", 0, 0.0, 0.0, '\0', false, false, false },
    make_int_type_info<uint8_t>("uint8",  'u', false, true),
    make_int_type_info<int8_t>("int8",    'i', true,  false),
    make_int_type_info<uint16_t>("uint16", 'u', false, true),
    make_int_type_info<int16_t>("int16",   'i', true,  false),
    make_int_type_info<uint32_t>("uint32", 'u', false, true),
    make_int_type_info<int32_t>("int32",   'i', true,  false),
    make_int_type_info<uint64_t>("uint64", 'u', false, true),
    make_int_type_info<int64_t>("int64",   'i', true,  false),
    { "float16", 2, -65504.0, 65504.0, 'f', false, false, true },
    make_float_type_info<float>("float32"),
    make_float_type_info<double>("float64")
}};

static_assert((size_t) Type::Float64 + 1 == TypeInfos.size(),
              "TypeInfos must mirror the Type enum order!");

/// Return the \ref TypeInfo record for \c type, raising on an invalid type.
inline const TypeInfo &type_info(Type type) {
    size_t index = (size_t) type;
    if (index >= TypeInfos.size())
        raise("type_info(): invalid field type!");
    return TypeInfos[index];
}

/// Convert a NumPy dtype kind/size pair to a supported Type, or Invalid.
inline Type type_from_numpy_kind_size(char kind, size_t size) {
    for (size_t i = (size_t) Type::UInt8; i < TypeInfos.size(); ++i) {
        const TypeInfo &info = TypeInfos[i];
        if (info.numpy_kind == kind && info.size == size)
            return (Type) i;
    }
    return Type::Invalid;
}

/// Copy a field's default value (\c type_size(type) bytes) from \c source into
/// the low bytes of the 64-bit \c target slot; zero-fills when \c source is null.
inline void copy_value(uint64_t &target, const void *source, Type type) {
    target = 0;
    if (source)
        memcpy(&target, source, type_size(type));
}

/// Mask \c value to the low bytes that are meaningful for \c type.
inline uint64_t canonical_value(uint64_t value, Type type) {
    size_t size = type_size(type);
    return size == 8 ? value : value & ((uint64_t(1) << (size * 8)) - 1);
}

/// Return the byte offset one past the last byte occupied by \c field.
inline size_t field_end(const Field &field) {
    size_t size = type_size(field.type);
    if (field.offset > std::numeric_limits<size_t>::max() - size)
        raise("field_end(): field \"" + field.name + "\" offset overflows!");
    return field.offset + size;
}

/// 2^e as a double (exact for the 0..64 exponents used here). constexpr so the
/// clamp bounds below fold to compile-time constants when the types are known.
constexpr double pow2(int e) {
    double r = 1.0;
    while (e-- > 0)
        r *= 2.0;
    return r;
}

/// Largest value that is (a) an exact integer in a float with \c mant_bits
/// mantissa bits and (b) <= the integer type's max, given the type's value-bit
/// count \c value_bits (excluding sign) and its \c type_max.
///
/// If the max fits the mantissa it is used as-is; otherwise the max (2^N - 1)
/// rounds up past the range, so we take the largest representable value below
/// 2^N, which is the exact integer 2^N - 2^(N - mant_bits). Clamping to this
/// before a float -> int convert means round -> clamp -> convert can never
/// overflow on either backend.
constexpr double int_clamp_hi(int value_bits, int mant_bits, double type_max) {
    return value_bits <= mant_bits
               ? type_max
               : pow2(value_bits) - pow2(value_bits - mant_bits);
}

/// Compile-time clamp bounds for a float -> int store, for the destination
/// integer type \c Int and working float type \c Float. Used by the software
/// fallback, where both types are statically known.
template <typename Int, typename Float>
constexpr std::pair<double, double> int_clamp_bounds() {
    return { (double) std::numeric_limits<Int>::lowest(),
             int_clamp_hi(std::numeric_limits<Int>::digits,
                          std::numeric_limits<Float>::digits,
                          (double) std::numeric_limits<Int>::max()) };
}

/// Runtime clamp bounds for the JIT, which only knows the types as \ref Type
/// values. Shares \ref int_clamp_hi with the templated fallback version, so the
/// two backends compute bit-identical bounds. \c working is \c Type::Float32 or
/// \c Type::Float64.
inline std::pair<double, double> int_clamp_bounds(Type dst, Type working) {
    const TypeInfo &info = type_info(dst);
    int value_bits = (int) info.size * 8 - (info.signed_integer ? 1 : 0);
    int mant_bits = working == Type::Float32 ? std::numeric_limits<float>::digits
                                             : std::numeric_limits<double>::digits;
    return { info.min_value,
             int_clamp_hi(value_bits, mant_bits, info.max_value) };
}

/// Encode a field's logical \ref Field::value into the raw bits stored for it
/// (native-order low bytes of a \c uint64_t), as the converters would write it.
inline uint64_t encode_value(const Field &field) {
    const TypeInfo &info = type_info(field.type);
    double value = field.value;

    if (has_flag(field.flags, Flag::Gamma))
        value = linear_to_srgb(value);

    uint64_t bits = 0;
    if (info.signed_integer || info.unsigned_integer) {
        if (has_flag(field.flags, Flag::Normalized))
            value *= info.max_value;
        if (value < info.min_value) value = info.min_value;
        if (value > info.max_value) value = info.max_value;
        int64_t iv = (int64_t) std::rint(value);
        memcpy(&bits, &iv, info.size);
    } else {
        switch (field.type) {
            case Type::Float16: { half  h((float) value); memcpy(&bits, &h, 2); } break;
            case Type::Float32: { float f = (float) value; memcpy(&bits, &f, 4); } break;
            case Type::Float64: { memcpy(&bits, &value, 8); } break;
            default: break;
        }
    }
    return bits;
}

NAMESPACE_END(struct_jit)
