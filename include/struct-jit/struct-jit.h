#pragma once

#include "fwd.h"
#include <cstddef>
#include <string>
#include <string_view>
#include <iosfwd>
#include <vector>
#include <utility>

#if defined(_MSC_VER)
#  if defined(SJIT_BUILD)
#    define SJIT_EXPORT    __declspec(dllexport)
#  else
#    define SJIT_EXPORT    __declspec(dllimport)
#  endif
#else
#  define SJIT_EXPORT    __attribute__ ((visibility("default")))
#endif

#if defined(_MSC_VER)
// C4251: STL members of exported classes lack a DLL interface
#  pragma warning(push)
#  pragma warning(disable: 4251)
#endif

NAMESPACE_BEGIN(struct_jit)

// --------------------------------------------------------------------------
// Byte order

/// Byte order of the fields in the \c Struct
enum class ByteOrder : uint32_t {
    Native,
    LittleEndian,
    BigEndian
};

extern SJIT_EXPORT ByteOrder native_byte_order();

// --------------------------------------------------------------------------
// Field types and type queries

/// List of field types supported by Struct-JIT
enum class Type : uint32_t {
    Invalid,

    // Signed and unsigned integer values
    UInt8,  Int8,
    UInt16, Int16,
    UInt32, Int32,
    UInt64, Int64,

    // Floating point values
    Float16, Float32, Float64
};

/// Check whether the given type is a signed integer
extern SJIT_EXPORT bool type_is_signed_int(Type type);

/// Check whether the given type is an unsigned integer
extern SJIT_EXPORT bool type_is_unsigned_int(Type type);

/// Check whether the given type is an integer (signed or unsigned)
extern SJIT_EXPORT bool type_is_integer(Type type);

/// Check whether the given type is a floating point type
extern SJIT_EXPORT bool type_is_float(Type type);

/// Check whether the given type is a signed type
extern SJIT_EXPORT bool type_is_signed(Type type);

/// Return the size in bytes of the given variable type
extern SJIT_EXPORT size_t type_size(Type type);

/// Return the representable range of a particular type
extern SJIT_EXPORT std::pair<double, double> type_range(Type type);

// --------------------------------------------------------------------------
// Compile-time mapping from a C++ scalar type to its \ref Type

template <typename T> struct type_id;

#define SJIT_TYPE_ID(T, entry)                                                 \
    template <> struct type_id<T> {                                            \
        static constexpr Type value = Type::entry;                             \
    };

SJIT_TYPE_ID(int8_t,   Int8)
SJIT_TYPE_ID(uint8_t,  UInt8)
SJIT_TYPE_ID(int16_t,  Int16)
SJIT_TYPE_ID(uint16_t, UInt16)
SJIT_TYPE_ID(int32_t,  Int32)
SJIT_TYPE_ID(uint32_t, UInt32)
SJIT_TYPE_ID(int64_t,  Int64)
SJIT_TYPE_ID(uint64_t, UInt64)
SJIT_TYPE_ID(float,    Float32)
SJIT_TYPE_ID(double,   Float64)
#undef SJIT_TYPE_ID

/// Convenience variable template: the \ref Type corresponding to C++ type \c T
template <typename T> constexpr Type type_v = type_id<T>::value;

// --------------------------------------------------------------------------
// Field flags

/// Optional flags that can be applied to each field
enum class Flag : uint32_t {
    /**
     * The integral field encodes a quantized value in the range [0, 1]. An
     * error is raised if this flag is specified for a floating point-typed
     * field.
     */
    Normalized = 1,

    /**
     * The field encodes a sRGB gamma-corrected value. Requires that
     * \c Normalized is also specified.
     */
    Gamma = 2,

    /**
     * When converting, check that the field value matches the specified
     * default value. Otherwise, return a conversion failure.
     */
    Check = 4,

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
     * The field stores an alpha value. When both the input and output declare an
     * alpha field, all other fields can be converted between premultiplied and
     * non-premultiplied representations (see \c PremultipliedAlpha).
     */
    Alpha = 32,

    /**
     * The field is premultiplied by the value of the \c Alpha field. Converting
     * between an input and output that disagree on this flag multiplies the
     * field by the alpha (premultiply) or its reciprocal (unpremultiply).
     */
    PremultipliedAlpha = 64
};

constexpr uint32_t operator |(Flag f1, Flag f2)     { return (uint32_t) f1 | (uint32_t) f2; }
constexpr uint32_t operator |(uint32_t f1, Flag f2) { return f1 | (uint32_t) f2; }
constexpr uint32_t operator &(Flag f1, Flag f2)     { return (uint32_t) f1 & (uint32_t) f2; }
constexpr uint32_t operator &(uint32_t f1, Flag f2) { return f1 & (uint32_t) f2; }
constexpr uint32_t operator ~(Flag f1)              { return ~(uint32_t) f1; }
constexpr uint32_t operator +(Flag e)               { return (uint32_t) e; }
constexpr bool has_flag(uint32_t flags, Flag f)     { return (flags & (uint32_t) f) != 0; }

// --------------------------------------------------------------------------
// Field

/// Specifies a single field of a \ref Struct instance
struct SJIT_EXPORT Field {
    /// Name of the field
    std::string name;

    /**
     * \brief Optional input field name for a plain renamed copy (target only).
     *
     * When non-empty, the converter reads this output from the input field named
     * \c source instead of one matching \ref name, with the usual type, gamma, and
     * alpha handling. Mutually exclusive with \ref blend. When left empty, the
     * converter falls back to looking up an input field matching \ref name.
     */
    std::string source;

    Type type = Type::Invalid;

    /// Offset within the \c Struct (in bytes)
    size_t offset = 0;

    /// Bitwise combination of \ref Flag values
    uint32_t flags = 0;

    /**
     * \brief Logical value associated with the field, in double precision.
     *
     * Used as the substituted default for a \ref Flag::Default field, or the
     * expected value for a \ref Flag::Check field. It is interpreted like a
     * regular field value, i.e. subject to the field's normalization and gamma
     * encoding.
     */
    double value = 0.0;

    /**
     * \brief Blend specification: when non-empty, the field's value is a linear
     * combination of other (named) input fields rather than a direct copy.
     *
     * Each entry is a \c (weight, source field name) pair; the output is
     * <tt>sum(weight_i * linearize(source_i))</tt>, evaluated in the converter's
     * working precision.
     */
    std::vector<std::pair<double, std::string>> blend;
};


// --------------------------------------------------------------------------
// Struct

/**
 * \brief Describes the in-memory layout of a record as an ordered list of
 * named, typed \ref Field entries.
 *
 * A \c Struct is the schema that a \ref Converter reads from and writes to. It
 * tracks each field's type, byte offset, optional \ref Flag annotations, and
 * default value, along with structure-wide properties such as the byte order
 * and whether fields are tightly packed or padded for natural alignment.
 */
class SJIT_EXPORT Struct {
public:
    using FieldIterator      = std::vector<Field>::iterator;
    using ConstFieldIterator = std::vector<Field>::const_iterator;

    /**
     * \brief Create an empty data structure
     *
     * \param pack
     *    If \c true, fields will be tightly packed without adding
     *    alignment-related padding
     *
     * \param byte_order
     *    Enables overriding the byte order of the data structure. If needed,
     *    Struct-JIT will perform endianness conversion during conversions.
     */
    Struct(bool pack = false, ByteOrder byte_order = ByteOrder::Native);

    /// Standard constructor/copy assignment/..
    Struct(const Struct &s) = default;
    Struct(Struct &&s) = default;
    Struct &operator=(const Struct &s) = default;
    Struct &operator=(Struct &&s) = default;

    /// Append a new field, while determining size and offset automatically
    Struct &append(const std::string &name,
                   Type type,
                   uint32_t flags = 0,
                   double value = 0.0);

    /// Append a new field to the \c Struct (all information must be provided)
    Struct &append(const Field &field);

    /// Are appended fields tightly packed (i.e. without alignment padding)?
    bool pack() const { return m_pack; }

    void set_pack(bool value) { m_pack = value; }

    /// Return the byte order of the \c Struct
    ByteOrder byte_order() const { return m_byte_order; }

    void set_byte_order(ByteOrder value);

    /// Return the total size (in bytes) of the data structure, including padding
    size_t nbytes() const;

    /// Return the alignment (in bytes) of the data structure
    size_t align() const;

    /// Validate field definitions and shared backend layout limits
    void validate() const;

    /// Return the number of fields
    size_t size() const { return m_fields.size(); }

    /// Check if the \c Struct has a field of the specified name
    bool contains(std::string_view name) const;

    /// Return an iterator that points to a field with the specified name [const]
    ConstFieldIterator find(std::string_view name) const;

    /// Return an iterator that points to a field with the specified name
    FieldIterator find(std::string_view name);

    /// Access an individual field by index [const]
    const Field &operator[](size_t i) const { return m_fields[i]; }

    /// Access an individual field by index
    Field &operator[](size_t i) { return m_fields[i]; }

    /// Access an individual field by name [const]
    const Field &operator[](const std::string &name) const;

    /// Access an individual field by name
    Field &operator[](const std::string &name);

    /// Return an iterator associated with the first field
    ConstFieldIterator begin() const { return m_fields.cbegin(); }

    /// Return an iterator associated with the first field
    FieldIterator begin() { return m_fields.begin(); }

    /// Return an iterator associated with the end of the data structure [const]
    ConstFieldIterator end() const { return m_fields.cend(); }

    /// Return an iterator associated with the end of the data structure
    FieldIterator end() { return m_fields.end(); }

private:
    bool m_pack;
    ByteOrder m_byte_order;
    std::vector<Field> m_fields;
};
// --------------------------------------------------------------------------
// Converter

/**
 * \brief Converts records from one \ref Struct layout to another.
 *
 * Given an input and output \ref Struct, the \c Converter analyzes the two
 * layouts and builds a plan that maps each output field to a matching input
 * field (or a default value). When \c jit is enabled, it compiles this plan
 * into a native machine-code kernel; otherwise a portable software fallback
 * performs the same conversion. A single \c Converter can be reused to convert
 * many elements (e.g. a 2D image) in one \ref convert() call.
 */
class SJIT_EXPORT Converter {
public:
    /**
     * \param working_precision
     *    Floating point precision used for the numeric conversion steps. A
     *    single working type is used for all fields; must be \c Type::Float32
     *    (the default) or \c Type::Float64.
     */
    Converter(const Struct &source, const Struct &target, bool jit = true,
              bool dither = false, Type working_precision = Type::Float32);
    ~Converter();

    Converter(const Converter &) = delete;
    Converter &operator=(const Converter &) = delete;
    Converter(Converter &&c) noexcept;
    Converter &operator=(Converter &&c) noexcept;

    const Struct &source() const { return m_source; }
    const Struct &target() const { return m_target; }

    bool convert(const void *in, void *out, size_t width, size_t height) const;

    /// Return the generated kernel machine code (e.g. for disassembly), or
    /// {nullptr, 0} when no kernel was compiled
    std::pair<const uint8_t *, size_t> kernel() const {
        return { (const uint8_t *) m_kernel, m_kernel_size };
    }

private:
    /// Analyze the input/output layouts and build \ref m_plan, the per-field
    /// transfer schedule shared by the JIT and software fallback.
    void create_plan();

    /// Compile \ref m_plan into a native machine-code kernel (\ref m_kernel).
    void create_kernel();

    /// Release the executable memory backing \ref m_kernel.
    void release_kernel();

    /// Convert a 2D region of records using the portable software interpreter.
    bool convert_fallback(const uint8_t *in, uint8_t *out, size_t width, size_t height) const;

    template <typename Float>
    bool convert_fallback_impl(const uint8_t *in, uint8_t *out, size_t width, size_t height) const;

private:
    using Kernel = bool (const void *, void *, size_t, size_t);
    Struct m_source, m_target;

    /// Ordered transfer schedule produced by \ref create_plan(). Each entry is
    /// a (input field index, output field index) pair; \ref None in the input
    /// slot marks a missing input (substitute the output default).
    std::vector<std::pair<size_t, size_t>> m_plan;

    /// Weighted -> unweighted: divide every mapped field by the source weight \ref m_weight_in.
    bool m_weight_divide = false;

    /// Source weight field index for \ref m_weight_divide (\c size_t(-1) if unused).
    size_t m_weight_in = size_t(-1);

    /// Both sides declare \ref Flag::Alpha, so fields can be (un)premultiplied by \ref m_alpha_in.
    bool m_alpha_apply = false;

    /// Source alpha field index for \ref m_alpha_apply (\c size_t(-1) if unused).
    size_t m_alpha_in = size_t(-1);

    /// One blended output field: its index plus the (input index, weight) terms summed into it.
    struct BlendEntry {
        size_t output;
        std::vector<std::pair<size_t, double>> terms;
    };

    /// Output fields computed as a linear combination of inputs (\ref Field::blend); applied after \ref m_plan.
    std::vector<BlendEntry> m_blend;

    /// Apply ordered dithering when quantizing float -> int (2D images).
    bool m_dither = false;

    /// Working precision (Float32 or Float64) used for every numeric conversion step.
    Type m_working = Type::Float32;

    /// Compiled kernel and its size in bytes (null when using the fallback).
    Kernel *m_kernel;
    size_t m_kernel_size;
};

// --------------------------------------------------------------------------
// Shared converter cache

/// Return an existing matching converter or create and cache one.
SJIT_EXPORT const Converter &make_converter(
    const Struct &source, const Struct &target, bool jit = true,
    bool dither = false, Type working_precision = Type::Float32);

/// Remove all cached converters and release their executable kernels.
SJIT_EXPORT void clear_cache();

// --------------------------------------------------------------------------
// Equality comparison operators

extern SJIT_EXPORT bool operator==(const Struct &f1, const Struct &f2);
extern SJIT_EXPORT bool operator!=(const Struct &f1, const Struct &f2);
extern SJIT_EXPORT bool operator==(const Field &f1, const Field &f2);
extern SJIT_EXPORT bool operator!=(const Field &f1, const Field &f2);

// --------------------------------------------------------------------------
// Stream insertion operators

extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Type &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const ByteOrder &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Flag &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Field &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Struct &);

NAMESPACE_END(struct_jit)

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
