#include "struct-jit.h"

Struct::Struct(bool pack, Struct::ByteOrder byte_order)
    : Object(), m_pack(pack), m_byte_order(byte_order) {
    if (m_byte_order == Struct::ByteOrder::HostByteOrder)
        m_byte_order = host_byte_order();
}

Struct::Struct(const Struct &s)
    : Object(), m_fields(s.m_fields), m_pack(s.m_pack),
      m_byte_order(s.m_byte_order) { }

size_t Struct::size() const {
    if (m_fields.empty())
        return 0;
    auto const &last = m_fields[m_fields.size() - 1];
    size_t size = last.offset + last.size;
    if (!m_pack) {
        size_t align = alignment();
        size += math::modulo(align - size, align);
    }
    return size;
}

size_t Struct::alignment() const {
    if (m_pack)
        return 1;
    size_t size = 1;
    for (auto const &field : m_fields)
        size = std::max(size, field.size);
    return size;
}

bool Struct::has_field(const std::string &name) const {
    for (auto const &field : m_fields)
        if (field.name == name)
            return true;
    return false;
}

Struct &Struct::append(const std::string &name, Struct::Type type, uint32_t flags, double default_) {
    Field f;
    f.name = name;
    f.type = type;
    f.flags = flags;
    f.default_ = default_;
    if (m_fields.empty()) {
        f.offset = 0;
    } else {
        auto const &last = m_fields[m_fields.size() - 1];
        f.offset = last.offset + last.size;
    }
    switch (type) {
        case Type::Int8:
        case Type::UInt8:   f.size = 1; break;
        case Type::Int16:
        case Type::UInt16:
        case Type::Float16: f.size = 2; break;
        case Type::Int32:
        case Type::UInt32:
        case Type::Float32: f.size = 4; break;
        case Type::Int64:
        case Type::UInt64:
        case Type::Float64: f.size = 8; break;
        default: Throw("Struct::append(): invalid field type!");
    }
    if (!m_pack)
        f.offset += math::modulo(f.size - f.offset, f.size);
    m_fields.push_back(f);
    return *this;
}

std::ostream &operator<<(std::ostream &os, Struct::Type value) {
    switch (value) {
        case Struct::Type::Int8:    os << "int8";    break;
        case Struct::Type::UInt8:   os << "uint8";   break;
        case Struct::Type::Int16:   os << "int16";   break;
        case Struct::Type::UInt16:  os << "uint16";  break;
        case Struct::Type::Int32:   os << "int32";   break;
        case Struct::Type::UInt32:  os << "uint32";  break;
        case Struct::Type::Int64:   os << "int64";   break;
        case Struct::Type::UInt64:  os << "uint64";  break;
        case Struct::Type::Float16: os << "float16"; break;
        case Struct::Type::Float32: os << "float32"; break;
        case Struct::Type::Float64: os << "float64"; break;
        case Struct::Type::Invalid: os << "invalid"; break;
        default: Throw("Struct: operator<<: invalid field type!");
    }
    return os;
}

std::string Struct::to_string() const {
    std::ostringstream os;
    os << "Struct<" << size() << ">[" << std::endl;
    for (size_t i = 0; i < m_fields.size(); ++i) {
        auto const &f = m_fields[i];
        if (i > 0) {
            size_t padding = f.offset - (m_fields[i-1].offset + m_fields[i-1].size);
            if (padding > 0)
                os << "  // " << padding << " byte" << (padding > 1 ? "s" : "") << " of padding." << std::endl;
        }
        os << "  " << f.type;
        os << " " << f.name << "; // @" << f.offset;
        if (has_flag(f.flags, Flags::Normalized))
            os << ", normalized";
        if (has_flag(f.flags, Flags::Gamma))
            os << ", gamma";
        if (has_flag(f.flags, Flags::Weight))
            os << ", weight";
        if (has_flag(f.flags, Flags::Alpha))
            os << ", alpha";
        if (has_flag(f.flags, Flags::PremultipliedAlpha))
            os << ", premultiplied alpha";
        if (has_flag(f.flags, Flags::Default))
            os << ", default=" << f.default_;
        if (has_flag(f.flags, Flags::Assert))
            os << ", assert=" << f.default_;
        if (!f.blend.empty()) {
            os << ", blend = <";
            for (size_t j = 0; j < f.blend.size(); ++j) {
                os << f.blend[j].second << " * " << f.blend[j].first;
                if (j + 1 < f.blend.size())
                    os << " + ";
            }
            os << ">";
        }
        os << "\n";
    }
    if (!m_fields.empty()) {
        size_t padding = size() - (m_fields[m_fields.size() - 1].offset +
                                   m_fields[m_fields.size() - 1].size);
        if (padding > 0)
            os << "  // " << padding << " byte" << (padding > 1 ? "s" : "") << " of padding." << std::endl;
    }
    os << "]";
    return os.str();
}

const Struct::Field &Struct::field(const std::string &name) const {
    for (auto const &field : m_fields)
        if (field.name == name)
            return field;
    Throw("Unable to find field \"%s\"", name);
}

Struct::Field &Struct::field(const std::string &name) {
    for (auto &field : m_fields)
        if (field.name == name)
            return field;
    Throw("Unable to find field \"%s\"", name);
}

std::pair<double, double> Struct::range(Struct::Type type) {
    std::pair<double, double> result;

    #define COMPUTE_RANGE(key, type)                                            \
        case key:                                                               \
            result = std::make_pair((double) std::numeric_limits<type>::min(),  \
                                    (double) std::numeric_limits<type>::max()); \
            break;

    switch (type) {
        COMPUTE_RANGE(Type::UInt8, uint8_t);
        COMPUTE_RANGE(Type::Int8, int8_t);
        COMPUTE_RANGE(Type::UInt16, uint16_t);
        COMPUTE_RANGE(Type::Int16, int16_t);
        COMPUTE_RANGE(Type::UInt32, uint32_t);
        COMPUTE_RANGE(Type::Int32, int32_t);
        COMPUTE_RANGE(Type::UInt64, uint64_t);
        COMPUTE_RANGE(Type::Int64, int64_t);
        COMPUTE_RANGE(Type::Float32, float);
        COMPUTE_RANGE(Type::Float64, double);

        case Type::Float16:
            result = std::make_pair(-65504, 65504);
            break;

        default:
            Throw("Internal error: invalid field type");
    }

    if (is_integer(type)) {
        // Account for rounding errors in the conversions above.
        // (we want the bounds to be conservative)
        if (result.first != 0)
            result.first = ek::next_float(result.first);
        result.second = ek::prev_float(result.second);
    }

    return result;
}

size_t hash(const Struct::Field &f) {
    size_t value = hash(f.name);
    value = hash_combine(value, hash(f.type));
    value = hash_combine(value, hash(f.size));
    value = hash_combine(value, hash(f.offset));
    value = hash_combine(value, hash(f.flags));
    value = hash_combine(value, hash(f.default_));
    return value;
}

size_t hash(const Struct &s) {
    return hash_combine(hash_combine(hash(s.m_fields), hash(s.m_pack)),
                        hash(s.m_byte_order));
}
