#include "fwd.h"
#include <string>

NAMESPACE_BEGIN(sjit)

/// Type of a field in the \c Struct
enum class Type : uint32_t {
    Unknown = 0,

    // Signed and unsigned integer values
    UInt8,  Int8,
    UInt16, Int16,
    UInt32, Int32,
    UInt64, Int64,

    // Floating point values
    Float16, Float32, Float64
};

/// Field-specific flags
enum class Flags : uint32_t {
    /// No flags set (default value)
    None = 0x00,

    /**
     * Specifies whether an integer field encodes a normalized value in the
     * range [0, 1]. The flag is ignored if specified for floating point
     * valued fields.
     */
    Normalized = 0x01,

    /**
     * Specifies whether the field encodes a sRGB gamma-corrected value.
     * Assumes \c Normalized is also specified.
     */
    Gamma      = 0x02,

    /**
     * In \ref FieldConverter::convert, check that the field value matches
     * the specified default value. Otherwise, return a failure
     */
    Assert     = 0x04,

    /**
     * In \ref FieldConverter::convert, when the field is missing in the
     * source record, replace it by the specified default value
     */
    Default    = 0x08,

    /**
     * In \ref FieldConverter::convert, when an input structure contains a
     * weight field, the value of all entries are considered to be
     * expressed relative to its value. Converting to an un-weighted
     * structure entails a division by the weight.
     */
    Weight     = 0x10,

    /**
     * Specifies whether the field encodes a color value with premultiplied
     * alpha. In this case, another field with the 'Alpha' flag must be
     * present.
     */
    PremultipliedAlpha = 0x20,

    /**
     * Specifies whether the field encodes an alpha value
    */
    Alpha      = 0x40
};

/// Byte order of the fields in the \c Struct
enum class ByteOrder {
    Default,
    LittleEndian,
    BigEndian
};

struct Field {
    std::string name;
    const void *ptr = nullptr;
    Type type = Type::Unknown;
    uint32_t flags = 0;
    uint32_t offset = 0;
};

NAMESPACE_END(sjit)
