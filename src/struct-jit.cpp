#include <struct-jit/struct-jit.h>
#include <algorithm>
#include <ostream>
#include <cstring>
#include <stdexcept>

NAMESPACE_BEGIN(struct_jit)

[[noreturn]] static void raise(const std::string &msg) {
    throw std::runtime_error(msg);
}

Struct::Struct(bool pack, ByteOrder byte_order)
    : m_pack(pack), m_byte_order(byte_order) {
    if (m_byte_order == ByteOrder::Native)
        m_byte_order = ByteOrder::LittleEndian;
}

void Struct::set_byte_order(ByteOrder value) {
    if (value == ByteOrder::Native)
        value = ByteOrder::LittleEndian;
    m_byte_order = value;
}

size_t Struct::align() const {
    if (m_pack)
        return 1;
    size_t size = 1;
    for (const Field &field : m_fields)
        size = std::max(size, (size_t) struct_jit::size(field.type));
    return size;
}

size_t Struct::size() const {
    if (m_fields.empty())
        return 0;
    const Field &last = m_fields[m_fields.size() - 1];
    size_t size = last.offset + struct_jit::size(last.type);
    if (!m_pack) {
        size_t a = align();
        size = (size + a - 1) / a * a;
    }
    return size;
}

Struct &Struct::append(const std::string &name, Type type, uint32_t flags, const void *value) {
    Field f { name, type, 0, flags, 0 };
    memcpy(&f.value, value, struct_jit::size(type));

    if (name.empty())
        raise("Struct::append(): a field name must be specified!");
    else if (contains(name))
        raise("Struct::append(): a field with the name \"" + name +
              "\" already exists!");

    if (!m_fields.empty()) {
        const Field &l = m_fields.back();
        f.offset = l.offset + struct_jit::size(l.type);
    }

    if (!m_pack) {
        size_t align = struct_jit::size(f.type);
        f.offset = (f.offset + align - 1) / align * align;
    }

    m_fields.push_back(f);
    return *this;
}

Struct &Struct::append(const Field &field) {
    if (field.name.empty())
        raise("Struct::append(): a field name must be specified!");
    else if (contains(field.name))
        raise("Struct::append(): a field with the name \"" + field.name +
              "\" already exists!");

    m_fields.push_back(field);
    return *this;
}

Struct::FieldIterator Struct::find(const std::string &name) {
    return std::find_if(
        m_fields.begin(),
        m_fields.end(),
        [&](const Field &f) { return f.name == name; }
    );
}

Struct::ConstFieldIterator Struct::find(const std::string &name) const {
    return std::find_if(
        m_fields.begin(),
        m_fields.end(),
        [&](const Field &f) { return f.name == name; }
    );
}

bool Struct::contains(const std::string &name) const {
    for (const Field &field : m_fields)
        if (field.name == name)
            return true;
    return false;
}

const Field &Struct::operator[](const std::string &name) const {
    for (const Field &field : m_fields)
        if (field.name == name)
            return field;
    raise("Struct::operator[]: unable to find entry \"" + name + "\"");
}

Field &Struct::operator[](const std::string &name) {
    for (Field &field : m_fields)
        if (field.name == name)
            return field;
    raise("Struct::operator[]: unable to find entry \"" + name + "\"");
}

std::ostream &operator<<(std::ostream &os, const Struct &v) {
    os << "Struct[" << std::endl;
    for (size_t i = 0; i < v.fields(); ++i) {
        const Field &f = v[i];
        os << "  " << f << std::endl;

        size_t p0  = f.offset + struct_jit::size(f.type),
               p1  = i + 1 < v.fields() ? v[i + 1].offset : v.size(),
               pad = p1 - p0;

        if (pad > 0)
            os << "  // " << pad << " byte" << (pad > 1 ? "s" : "") << " of padding" << std::endl;
    }
    os << "]";
    return os;
}

// --------------------------------------------------------------------------

bool is_signed_int(Type type) {
    return type == Type::Int8  || type == Type::Int16 ||
           type == Type::Int32 || type == Type::Int64;
}

bool is_unsigned_int(Type type) {
    return type == Type::UInt8  || type == Type::UInt16 ||
           type == Type::UInt32 || type == Type::UInt64;
}

bool is_float(Type type) {
    return type == Type::Float16 || type == Type::Float32 || type == Type::Float64;
}

size_t size(Type type) {
    switch (type) {
        case Type::Int8:
        case Type::UInt8:   return 1;
        case Type::Int16:
        case Type::UInt16:
        case Type::Float16: return 2;
        case Type::Int32:
        case Type::UInt32:
        case Type::Float32: return 4;
        case Type::Int64:
        case Type::UInt64:
        case Type::Float64: return 8;
        default:
            raise("size(): invalid field type!");
    }
}

// --------------------------------------------------------------------------

bool operator==(const Field &f1, const Field &f2) {
    return f1.name == f2.name &&
           f1.type == f2.type &&
           f1.offset == f2.offset &&
           f1.flags == f2.flags &&
           memcmp(&f1.value, &f2.value, size(f1.type)) == 0;
}

bool operator!=(const Field &f1, const Field &f2) {
    return !operator==(f1, f2);
}

bool operator==(const Struct &s1, const Struct &s2) {
    if (s1.pack() != s2.pack() ||
        s1.byte_order() != s2.byte_order() ||
        s1.fields() != s2.fields())
        return false;

    for (size_t i = 0; i < s1.fields(); ++i) {
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
    switch (v) {
        case Type::Invalid: os << "invalid"; break;
        case Type::Int8:    os << "int8";    break;
        case Type::UInt8:   os << "uint8";   break;
        case Type::Int16:   os << "int16";   break;
        case Type::UInt16:  os << "uint16";  break;
        case Type::Int32:   os << "int32";   break;
        case Type::UInt32:  os << "uint32";  break;
        case Type::Int64:   os << "int64";   break;
        case Type::UInt64:  os << "uint64";  break;
        case Type::Float16: os << "float16"; break;
        case Type::Float32: os << "float32"; break;
        case Type::Float64: os << "float64"; break;
        default: raise("operator<<(Type): invalid field type!");
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Flag &v) {
    switch (v) {
        case Flag::Normalized: os << "normalized"; break;
        case Flag::Gamma:      os << "gamma";      break;
        case Flag::Check:      os << "check";      break;
        case Flag::Default:    os << "default";    break;
        case Flag::Weight:     os << "weight";     break;
        default: raise("operator<<(Flag): invalid field type!");
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const Field &v) {
    os << v.type << " " << v.name << "; // @" << v.offset;
    for (Flag f : { Flag::Normalized, Flag::Gamma, Flag::Weight, Flag::Default, Flag::Check }) {
       if (has_flag(v.flags, f))
           os << ", " << f;
    }

    if (has_flag(v.flags, Flag::Default) ||
        has_flag(v.flags, Flag::Check))
        os << ", value=0x" << std::hex << v.value << std::hex;

    return os;
}


NAMESPACE_END(struct_jit)
