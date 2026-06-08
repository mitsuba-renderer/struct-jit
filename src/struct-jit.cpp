#include <struct-jit/struct-jit.h>
#include "type_info.h"
#include <algorithm>
#include <limits>
#include <ostream>
#include <sstream>

NAMESPACE_BEGIN(struct_jit)

static constexpr uint32_t ValidFieldFlags =
    +Flag::Normalized |
    +Flag::Gamma |
    +Flag::Check |
    +Flag::Default |
    +Flag::Weight |
    +Flag::Alpha |
    +Flag::PremultipliedAlpha;

static std::string format_hex(uint32_t value) {
    std::ostringstream oss;
    oss << "0x" << std::hex << value;
    return oss.str();
}

static void validate_field_definition(const Field &f) {
    if (f.name.empty())
        raise("Struct::validate(): a field name must be specified!");
    if (f.type == Type::Invalid)
        raise("Struct::validate(): field \"" + f.name +
              "\" has an invalid type!");

    uint32_t unknown = f.flags & ~ValidFieldFlags;
    if (unknown)
        raise("Struct::validate(): field \"" + f.name +
              "\" contains unknown flag bits (" + format_hex(unknown) + ")!");

    if (has_flag(f.flags, Flag::Normalized) && type_is_float(f.type))
        raise("Struct::validate(): field \"" + f.name +
              "\" specifies 'Normalized', which requires an integral type!");

    if (has_flag(f.flags, Flag::Gamma) && type_is_integer(f.type) &&
        !has_flag(f.flags, Flag::Normalized))
        raise("Struct::validate(): integer field \"" + f.name +
              "\" specifies 'Gamma' without 'Normalized'!");

    if (has_flag(f.flags, Flag::Check) && has_flag(f.flags, Flag::Default))
        raise("Struct::validate(): field \"" + f.name +
              "\" specifies mutually exclusive 'Check' and 'Default' flags!");

    (void) field_end(f);
}

ByteOrder native_byte_order() {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return ByteOrder::BigEndian;
#else
    return ByteOrder::LittleEndian;
#endif
}

Struct::Struct(bool pack, ByteOrder byte_order)
    : m_pack(pack), m_byte_order(byte_order) {
    if (m_byte_order == ByteOrder::Native)
        m_byte_order = native_byte_order();
}

void Struct::set_byte_order(ByteOrder value) {
    if (value == ByteOrder::Native)
        m_byte_order = native_byte_order();
    else
        m_byte_order = value;
}

size_t Struct::align() const {
    if (m_pack)
        return 1;
    size_t size = 1;
    for (const Field &field : m_fields)
        size = std::max(size, type_size(field.type));
    return size;
}

size_t Struct::nbytes() const {
    if (m_fields.empty())
        return 0;
    size_t size = 0;
    for (const Field &field : m_fields)
        size = std::max(size, field_end(field));
    if (!m_pack) {
        size_t a = align();
        if (size > std::numeric_limits<size_t>::max() - (a - 1))
            raise("Struct::nbytes(): data structure size overflows!");
        size = (size + a - 1) / a * a;
    }
    return size;
}

Struct &Struct::append(const std::string &name, Type type, uint32_t flags, double value) {
    if (type == Type::Invalid)
        raise("Struct::append(): an invalid field type was specified!");

    Field f { name, type, 0, flags, value, {} };
    validate_field_definition(f);

    if (contains(name))
        raise("Struct::append(): a field with the name \"" + name +
              "\" already exists!");

    if (!m_fields.empty()) {
        const Field &l = m_fields.back();
        f.offset = field_end(l);
    }

    if (!m_pack) {
        size_t align = type_size(f.type);
        f.offset = (f.offset + align - 1) / align * align;
    }

    m_fields.push_back(f);
    return *this;
}

Struct &Struct::append(const Field &field_) {
    Field field = field_;
    validate_field_definition(field);

    if (contains(field.name))
        raise("Struct::append(): a field with the name \"" + field.name +
              "\" already exists!");

    auto it = std::lower_bound(
        m_fields.begin(),
        m_fields.end(),
        field.offset,
        [](const Field &candidate, size_t offset) {
            return candidate.offset < offset;
        }
    );

    // Keep manually positioned fields sorted by offset so nbytes(), padding
    // reporting, and later conversion planning all see one canonical layout.
    if (it != m_fields.begin()) {
        const Field &prev = *(it - 1);
        if (field.offset < field_end(prev))
            raise("Struct::append(): field \"" + field.name +
                  "\" overlaps with field \"" + prev.name + "\"!");
    }

    if (it != m_fields.end()) {
        if (field_end(field) > it->offset)
            raise("Struct::append(): field \"" + field.name +
                  "\" overlaps with field \"" + it->name + "\"!");
    }

    m_fields.insert(it, field);
    return *this;
}

void Struct::validate() const {
    size_t prev_end = 0;
    for (size_t i = 0; i < m_fields.size(); ++i) {
        const Field &f = m_fields[i];
        validate_field_definition(f);

        for (size_t j = 0; j < i; ++j) {
            if (m_fields[j].name == f.name)
                raise("Struct::validate(): field name \"" + f.name +
                      "\" appears more than once!");
        }

        size_t size = type_size(f.type),
               end  = field_end(f);

        if (i > 0 && f.offset < m_fields[i - 1].offset)
            raise("Struct::validate(): fields must be sorted by offset!");

        if (i > 0 && f.offset < prev_end)
            raise("Struct::validate(): field \"" + f.name +
                  "\" overlaps with field \"" + m_fields[i - 1].name + "\"!");

        if (end > MaxRecordBytes)
            raise("Struct::validate(): field \"" + f.name +
                  "\" exceeds the maximum supported record size (" +
                  std::to_string(MaxRecordBytes) + " bytes)!");

        if (size > 1 && (f.offset % size) != 0 &&
            f.offset > MaxUnalignedOffset)
            raise("Struct::validate(): field \"" + f.name +
                  "\" has an unaligned offset beyond the maximum shared backend "
                  "limit (" + std::to_string(MaxUnalignedOffset) + " bytes)!");

        prev_end = end;
    }

    size_t stride = nbytes();
    if (stride > MaxRecordBytes)
        raise("Struct::validate(): record stride exceeds the maximum supported "
              "shared backend limit (" + std::to_string(MaxRecordBytes) +
              " bytes)!");
}

Struct::FieldIterator Struct::find(std::string_view name) {
    return std::find_if(
        m_fields.begin(),
        m_fields.end(),
        [&](const Field &f) { return f.name == name; }
    );
}

Struct::ConstFieldIterator Struct::find(std::string_view name) const {
    return std::find_if(
        m_fields.begin(),
        m_fields.end(),
        [&](const Field &f) { return f.name == name; }
    );
}

bool Struct::contains(std::string_view name) const {
    return find(name) != end();
}

const Field &Struct::operator[](const std::string &name) const {
    auto it = find(name);
    if (it != end())
        return *it;
    raise("Struct::operator[]: unable to find entry \"" + name + "\"");
}

Field &Struct::operator[](const std::string &name) {
    auto it = find(name);
    if (it != end())
        return *it;
    raise("Struct::operator[]: unable to find entry \"" + name + "\"");
}

std::ostream &operator<<(std::ostream &os, const Struct &v) {
    os << "Struct[" << std::endl;
    for (size_t i = 0; i < v.size(); ++i) {
        const Field &f = v[i];
        os << "  " << f << std::endl;

        size_t p0  = field_end(f),
               p1  = i + 1 < v.size() ? v[i + 1].offset : v.nbytes(),
               pad = p1 - p0;

        if (pad > 0)
            os << "  // " << pad << " byte" << (pad > 1 ? "s" : "") << " of padding" << std::endl;
    }
    os << "]";
    return os;
}

// --------------------------------------------------------------------------

bool type_is_signed_int(Type type) {
    return type_info(type).signed_integer;
}

bool type_is_unsigned_int(Type type) {
    return type_info(type).unsigned_integer;
}

bool type_is_integer(Type type) {
    const TypeInfo &info = type_info(type);
    return info.signed_integer || info.unsigned_integer;
}

bool type_is_float(Type type) {
    return type_info(type).floating_point;
}

bool type_is_signed(Type type) {
    const TypeInfo &info = type_info(type);
    return info.signed_integer || info.floating_point;
}

size_t type_size(Type type) {
    return type_info(type).size;
}

std::pair<double, double> type_range(Type type) {
    const TypeInfo &info = type_info(type);
    return { info.min_value, info.max_value };
}

// --------------------------------------------------------------------------

bool operator==(const Field &f1, const Field &f2) {
    return f1.name == f2.name &&
           f1.type == f2.type &&
           f1.offset == f2.offset &&
           f1.flags == f2.flags &&
           f1.blend == f2.blend &&
           f1.value == f2.value;
}

bool operator!=(const Field &f1, const Field &f2) {
    return !operator==(f1, f2);
}

bool operator==(const Struct &s1, const Struct &s2) {
    if (s1.pack() != s2.pack() ||
        s1.byte_order() != s2.byte_order() ||
        s1.size() != s2.size())
        return false;

    for (size_t i = 0; i < s1.size(); ++i) {
        if (s1[i] != s2[i])
            return false;
    }

    return true;
}

bool operator!=(const Struct &s1, const Struct &s2) {
    return !operator==(s1, s2);
}

// --------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &os, const ByteOrder &v) {
    switch (v) {
        case ByteOrder::Native:        os << "native";        break;
        case ByteOrder::LittleEndian:  os << "little-endian"; break;
        case ByteOrder::BigEndian:     os << "big-endian";    break;
        default: raise("operator<<(ByteOrder): invalid field type!");
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Type &v) {
    os << type_info(v).name;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Flag &v) {
    switch (v) {
        case Flag::Normalized: os << "normalized"; break;
        case Flag::Gamma:      os << "gamma";      break;
        case Flag::Check:      os << "check";      break;
        case Flag::Default:    os << "default";    break;
        case Flag::Weight:     os << "weight";     break;
        case Flag::Alpha:      os << "alpha";      break;
        case Flag::PremultipliedAlpha: os << "premultiplied alpha"; break;
        default: raise("operator<<(Flag): invalid field type!");
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Field &v) {
    os << v.type << " " << v.name << "; // @" << v.offset;
    for (Flag f : { Flag::Normalized, Flag::Gamma, Flag::Weight, Flag::Alpha,
                    Flag::PremultipliedAlpha, Flag::Default, Flag::Check }) {
       if (has_flag(v.flags, f))
           os << ", " << f;
    }

    if (has_flag(v.flags, Flag::Default) ||
        has_flag(v.flags, Flag::Check))
        os << ", value=" << v.value;

    if (!v.blend.empty()) {
        os << ", blend=<";
        for (size_t i = 0; i < v.blend.size(); ++i) {
            os << v.blend[i].first << " * " << v.blend[i].second;
            if (i + 1 < v.blend.size())
                os << " + ";
        }
        os << ">";
    }

    return os;
}


NAMESPACE_END(struct_jit)
