#include "fwd.h"
#include <string>

NAMESPACE_BEGIN(struct_jit)

/// Byte order of the fields in the \c Struct
enum class ByteOrder {
    HostByteOrder,
    LittleEndian,
    BigEndian
};

/// List of field types supported by Struct-JIT
enum class Type : uint32_t {
    Invalid = 0,

    // Signed and unsigned integer values
    UInt8,  Int8,
    UInt16, Int16,
    UInt32, Int32,
    UInt64, Int64,

    // Floating point values
    Float16, Float32, Float64
};


/// Optional flags that can be applied to each field
enum class Flag : uint32_t {
    /**
     * The integral field encodes a quantized value in the range [0, 1].
     * Ignored on fields with a floating point type.
     */
    Normalized = 1,

    /**
     * The field encodes a sRGB gamma-corrected value. Assumes \c Normalized is
     * also specified.
     */
    Gamma = 2,

    /**
     * When convertinhg, check that the field value matches the specified
     * default value. Otherwise, return a conversion failure.
     */
    Assert = 4,

    /**
     * When the field is missing in the source record, replace it by the
     * specified default value.
     */
    Default = 8,

    /**
     * The field stores a weight value. All other struct members are considered
     * relative to this weight. Converting to an un-weighted structure thus
     * entails a division by the weight. Useful for physically based rendering,
     * where the weight tracks the accumulated contribution of Monte Carlo
     * samples.
     */
    Weight = 16,

    /**
     * The field encodes an alpha value
    */
    Alpha = 32,

    /**
     * The field encodes a color value with premultiplied alpha. In this case,
     * another field with the 'Alpha' flag must be present.
     */
    PremultipliedAlpha = 64
};

constexpr uint32_t operator |(Flag f1, Flag f2) { return (uint32_t) f1 | (uint32_t) f2; }
constexpr uint32_t operator |(uint32_t f1, Flag f2)      { return f1 | (uint32_t) f2; }
constexpr uint32_t operator &(Flag f1, Flag f2) { return (uint32_t) f1 & (uint32_t) f2; }
constexpr uint32_t operator &(uint32_t f1, Flag f2)      { return f1 & (uint32_t) f2; }
constexpr uint32_t operator ~(Flag f1)                   { return ~(uint32_t) f1; }
constexpr uint32_t operator +(Flag e)                    { return (uint32_t) e; }
constexpr bool has_flag(uint32_t flags, Flag f)          { return (flags & (uint32_t) f) != 0; }

class Struct {


    struct Field {
        std::string name;
        const void *ptr = nullptr;
        Type type = Type::Invalid;
        uint32_t flags = 0;
        uint32_t offset = 0;
    };

};

NAMESPACE_END(struct_jit)
