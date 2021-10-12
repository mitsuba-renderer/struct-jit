#include "fwd.h"
#include <string>
#include <iosfwd>
#include <vector>

#if defined(_MSC_VER)
#  if defined(SJIT_BUILD)
#    define SJIT_EXPORT    __declspec(dllexport)
#  else
#    define SJIT_EXPORT    __declspec(dllimport)
#  endif
#else
#  define SJIT_EXPORT    __attribute__ ((visibility("default")))
#endif

NAMESPACE_BEGIN(struct_jit)

// --------------------------------------------------------------------------

/// Byte order of the fields in the \c Struct
enum class ByteOrder : uint32_t {
    Native,
    LittleEndian,
    BigEndian
};

extern SJIT_EXPORT ByteOrder native_byte_order();

// --------------------------------------------------------------------------

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
extern SJIT_EXPORT bool is_signed_int(Type type);

/// Check whether the given type is an unsigned integer
extern SJIT_EXPORT bool is_unsigned_int(Type type);

/// Check whether the given type is a floating point type
extern SJIT_EXPORT bool is_float(Type type);

/// Check whether the given type is a signed type
extern SJIT_EXPORT bool is_signed(Type type);

/// Return the size in bytes of the given variable type
extern SJIT_EXPORT size_t size(Type type);

/// Return the representable range of a particular type
extern SJIT_EXPORT std::pair<double, double> range(Type type);

// --------------------------------------------------------------------------

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
    Weight = 16
};

constexpr uint32_t operator |(Flag f1, Flag f2)     { return (uint32_t) f1 | (uint32_t) f2; }
constexpr uint32_t operator |(uint32_t f1, Flag f2) { return f1 | (uint32_t) f2; }
constexpr uint32_t operator &(Flag f1, Flag f2)     { return (uint32_t) f1 & (uint32_t) f2; }
constexpr uint32_t operator &(uint32_t f1, Flag f2) { return f1 & (uint32_t) f2; }
constexpr uint32_t operator ~(Flag f1)              { return ~(uint32_t) f1; }
constexpr uint32_t operator +(Flag e)               { return (uint32_t) e; }
constexpr bool has_flag(uint32_t flags, Flag f)     { return (flags & (uint32_t) f) != 0; }

// --------------------------------------------------------------------------

/// Specifies a single field of a \ref Struct instance
struct SJIT_EXPORT Field {
    /// Name of the field
    std::string name;

    /// Type identifier
    Type type = Type::Invalid;

    /// Offset within the \c Struct (in bytes)
    size_t offset = 0;

    /// Additional flags
    uint32_t flags = 0;

    /// Default value
    uint64_t value = 0;
};


// --------------------------------------------------------------------------

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

    /// Append a new field to the \c Struct; determines size and offset automatically
    Struct &append(const std::string &name,
                   Type type,
                   uint32_t flags = 0,
                   const void *value = nullptr);

    /// Append a new field to the \c Struct (manual version)
    Struct &append(const Field &field);

    /// Return whether entries appended to the \c Struct should be tightly packed?
    bool pack() const { return m_pack; }

    /// Specify whether entries appended to the \c Struct should be tightly packed?
    void set_pack(bool value) { m_pack = value; }

    /// Return the byte order of the \c Struct
    ByteOrder byte_order() const { return m_byte_order; }

    /// Specify the byte order of the \c Struct
    void set_byte_order(ByteOrder value);

    /// Return the size (in bytes) of the data structure, including padding
    size_t size() const;

    /// Return the alignment (in bytes) of the data structure
    size_t align() const;

    /// Return the number of fields
    size_t fields() const { return m_fields.size(); }

    /// Check if the \c Struct has a field of the specified name
    bool contains(const std::string &name) const;

    /// Return an iterator that points to a field with the specified name [const]
    ConstFieldIterator find(const std::string &name) const;

    /// Return an iterator that points to a field with the specified name
    FieldIterator find(const std::string &name);

    /// Access an individual field by index [const]
    const Field &operator[](size_t i) const { return m_fields[i]; }

    /// Access an individual field by index
    Field &operator[](size_t i) { return m_fields[i]; }

    /// Access an individual field by index [const]
    const Field &operator[](const std::string &name) const;

    /// Access an individual field by index
    Field &operator[](const std::string &name);

    /// Return an iterator associated with the first field
    ConstFieldIterator begin() const { return m_fields.cbegin(); }

    /// Return an iterator associated with the first field
    FieldIterator begin() { return m_fields.begin(); }

    /// Return an iterator associated with the end of the data structure [const]
    ConstFieldIterator end() const { return m_fields.cend(); }

    /// Return an iterator associated with the end of the data structure [const]
    FieldIterator end() { return m_fields.end(); }

private:
    bool m_pack;
    ByteOrder m_byte_order;
    std::vector<Field> m_fields;
};
// --------------------------------------------------------------------------


/**
 */
class SJIT_EXPORT Converter {
public:
    Converter(const Struct &in, const Struct &out, bool jit = true);

    const Struct &in() const { return m_in; }
    const Struct &out() const { return m_out; }

    bool convert(const void *in, void *out, size_t width, size_t height) const;

private:
    void create_plan();
    bool convert_slow(const uint8_t *in, uint8_t *out, size_t x, size_t y) const;

private:
    Struct m_in, m_out;
    std::vector<std::pair<size_t, size_t>> m_plan;
    bool m_jit;
};

// --------------------------------------------------------------------------

extern SJIT_EXPORT bool operator==(const Struct &f1, const Struct &f2);
extern SJIT_EXPORT bool operator!=(const Struct &f1, const Struct &f2);
extern SJIT_EXPORT bool operator==(const Field &f1, const Field &f2);
extern SJIT_EXPORT bool operator!=(const Field &f1, const Field &f2);

// --------------------------------------------------------------------------

extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Type &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const ByteOrder &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Flag &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Field &);
extern SJIT_EXPORT std::ostream &operator<<(std::ostream &, const Struct &);

NAMESPACE_END(struct_jit)
