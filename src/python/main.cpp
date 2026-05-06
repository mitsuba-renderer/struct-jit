#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/vector.h>
#include <struct-jit/struct-jit.h>
#include <sstream>
#include <cstring>
#include "../half.h"
#include "../type_info.h"
#include "docstr.h"

#define D(...) DOC(struct_jit, __VA_ARGS__)

namespace nb = nanobind;
namespace sj = struct_jit;
using namespace nb::literals;

using InputArray =
    nb::ndarray<nb::numpy, nb::ro, nb::c_contig, nb::device::cpu>;

static sj::Type type_from_dtype(nb::handle dt) {
    std::string kind_s = nb::cast<std::string>(dt.attr("kind"));
    size_t itemsize = nb::cast<size_t>(dt.attr("itemsize"));
    char kind = kind_s.empty() ? '\0' : kind_s[0];

    sj::Type type = sj::type_from_numpy_kind_size(kind, itemsize);
    if (type != sj::Type::Invalid)
        return type;

    throw nb::type_error("struct_jit.Type(): Invalid input type!");
}

static sj::ByteOrder byte_order_from_dtype(nb::handle dt) {
    nb::object byte_order = dt.attr("byteorder");

    if (byte_order.equal(nb::str(">")))
        return sj::ByteOrder::BigEndian;
    if (byte_order.equal(nb::str("<")))
        return sj::ByteOrder::LittleEndian;
    if (byte_order.equal(nb::str("=")))
        return sj::native_byte_order();

    // NumPy uses '|' for byte-order-independent types such as uint8.
    return sj::ByteOrder::Native;
}

static void merge_dtype_byte_order(sj::ByteOrder &record_order,
                                   sj::ByteOrder field_order) {
    if (field_order == sj::ByteOrder::Native)
        return;

    if (record_order == sj::ByteOrder::Native) {
        record_order = field_order;
    } else if (record_order != field_order) {
        throw nb::type_error(
            "struct_jit.Struct(): mixed-endian structured dtypes are not "
            "supported!");
    }
}

static void select_dtype_stride_mode(sj::Struct &s, size_t itemsize,
                                     size_t max_end) {
    if (itemsize < max_end)
        throw nb::type_error(
            "struct_jit.Struct(): structured dtype itemsize is smaller than "
            "the occupied field range!");

    // Candidate 1: the same offsets with Struct's natural trailing alignment.
    // Prefer this when both candidates match, because NumPy dtypes cannot encode
    // whether a naturally aligned no-padding layout came from pack=true or false.
    s.set_pack(false);
    if (s.nbytes() == itemsize)
        return;

    // Candidate 2: explicit dtype offsets with no trailing alignment padding.
    s.set_pack(true);
    if (s.nbytes() == itemsize)
        return;

    throw nb::type_error(
        "struct_jit.Struct(): structured dtype has trailing or "
        "non-representable padding; explicit record itemsize is not supported "
        "by struct_jit.Struct!");
}

static sj::Struct struct_from_dtype(nb::handle dt) {
    nb::object fields = dt.attr("fields");

    if (fields.is_none())
        throw nb::type_error("struct_jit.Struct(): input is not a structured dtype!");

    size_t itemsize = nb::cast<size_t>(dt.attr("itemsize")),
           max_end = 0;
    sj::Struct s;
    sj::ByteOrder byte_order = sj::ByteOrder::Native;

    for (nb::handle key: fields) {
        nb::tuple value = nb::cast<nb::tuple>(fields[key]);
        nb::object sub_dt = value[0];

        merge_dtype_byte_order(byte_order, byte_order_from_dtype(sub_dt));

        sj::Field f;
        f.name = nb::cast<std::string>(key);
        f.type = type_from_dtype(sub_dt);
        f.offset = nb::cast<size_t>(value[1]);

        size_t end = sj::field_end(f);
        if (end > max_end)
            max_end = end;

        // Append(Field) preserves the explicit dtype offsets instead of
        // recomputing a packed/native layout from field order.
        s.append(f);
    }

    s.set_byte_order(byte_order);
    select_dtype_stride_mode(s, itemsize, max_end);
    return s;
}

template <typename T> static nb::object load_value(const sj::Field &f) {
    T value;
    memcpy(&value, &f.value, sj::type_size(f.type));
    return nb::cast(value);
}

template <typename T> static void store_value(sj::Field &f, nb::handle o) {
    T value = nb::cast<T>(o);
    f.value = 0;
    memcpy(&f.value, &value, sj::type_size(f.type));
}

static nb::object field_value_to_python(const sj::Field &f) {
    switch (f.type) {
        case sj::Type::UInt8:   return load_value<uint8_t>(f);
        case sj::Type::Int8:    return load_value<int8_t>(f);
        case sj::Type::UInt16:  return load_value<uint16_t>(f);
        case sj::Type::Int16:   return load_value<int16_t>(f);
        case sj::Type::UInt32:  return load_value<uint32_t>(f);
        case sj::Type::Int32:   return load_value<int32_t>(f);
        case sj::Type::UInt64:  return load_value<uint64_t>(f);
        case sj::Type::Int64:   return load_value<int64_t>(f);
        case sj::Type::Float16: {
            uint16_t bits;
            memcpy(&bits, &f.value, sizeof(bits));
            return nb::cast(float16_to_float32(bits));
        }
        case sj::Type::Float32: return load_value<float>(f);
        case sj::Type::Float64: return load_value<double>(f);
        default:
            throw nb::type_error("Unexpected field type!");
    }
}

static void field_value_from_python(sj::Field &f, nb::handle o) {
    switch (f.type) {
        case sj::Type::UInt8:   store_value<uint8_t>(f, o); break;
        case sj::Type::Int8:    store_value<int8_t>(f, o); break;
        case sj::Type::UInt16:  store_value<uint16_t>(f, o); break;
        case sj::Type::Int16:   store_value<int16_t>(f, o); break;
        case sj::Type::UInt32:  store_value<uint32_t>(f, o); break;
        case sj::Type::Int32:   store_value<int32_t>(f, o); break;
        case sj::Type::UInt64:  store_value<uint64_t>(f, o); break;
        case sj::Type::Int64:   store_value<int64_t>(f, o); break;
        case sj::Type::Float16: {
            uint16_t bits = float32_to_float16((float) nb::cast<double>(o));
            f.value = 0;
            memcpy(&f.value, &bits, sizeof(bits));
            break;
        }
        case sj::Type::Float32: store_value<float>(f, o); break;
        case sj::Type::Float64: store_value<double>(f, o); break;
        default:
            throw nb::type_error("Unexpected field type!");
    }
}

static void object_to_value_bits(sj::Type type, nb::handle o, uint64_t &value) {
    sj::Field field;
    field.type = type;
    field_value_from_python(field, o);
    value = field.value;
}

static nb::bytes convert_buffer(const sj::Converter &c, const void *input_data,
                                size_t input_len, size_t height) {
    size_t record_size = c.in().nbytes();
    if (record_size == 0)
        throw std::runtime_error("Input structure has a record size of zero!");
    size_t count = input_len / record_size;
    if (count * record_size != input_len)
        throw std::runtime_error("Input length is not divisible by record size!");
    if (height == 0 || count % height != 0)
        throw std::runtime_error("Element count is not divisible by height!");

    // Records are laid out row-major; width is the fast (inner) axis.
    size_t width = count / height;
    std::string result(c.out().nbytes() * count, '\0');
    bool ok;
    {
        nb::gil_scoped_release release;
        ok = c.convert(input_data, (void *) result.data(), width, height);
    }
    if (!ok)
        throw std::runtime_error("Conversion failed!");

    return nb::bytes(result.data(), result.size());
}

NB_MODULE(struct_jit_ext, m_) {
    (void) m_;
    nb::module_ m = nb::module_::import_("struct_jit");
    m.attr("__doc__") = "Struct-JIT";

    nb::enum_<sj::ByteOrder>(m, "ByteOrder", D(ByteOrder))
        .value("Native", sj::ByteOrder::Native, D(ByteOrder, Native))
        .value("LittleEndian", sj::ByteOrder::LittleEndian, D(ByteOrder, LittleEndian))
        .value("BigEndian", sj::ByteOrder::BigEndian, D(ByteOrder, BigEndian));

    nb::enum_<sj::Type>(m, "Type", D(Type))
        .value("Invalid", sj::Type::Invalid, D(Type, Invalid))
        .value("Int8",    sj::Type::Int8,    D(Type, Int8))
        .value("UInt8",   sj::Type::UInt8,   D(Type, UInt8))
        .value("Int16",   sj::Type::Int16,   D(Type, Int16))
        .value("UInt16",  sj::Type::UInt16,  D(Type, UInt16))
        .value("Int32",   sj::Type::Int32,   D(Type, Int32))
        .value("UInt32",  sj::Type::UInt32,  D(Type, UInt32))
        .value("Int64",   sj::Type::Int64,   D(Type, Int64))
        .value("UInt64",  sj::Type::UInt64,  D(Type, UInt64))
        .value("Float16", sj::Type::Float16, D(Type, Float16))
        .value("Float32", sj::Type::Float32, D(Type, Float32))
        .value("Float64", sj::Type::Float64, D(Type, Float64));

    m.def("type_from_dtype", &type_from_dtype, "dtype"_a,
          "Convert a NumPy dtype into the equivalent struct_jit.Type");

    nb::enum_<sj::Flag>(m, "Flag", nb::is_arithmetic(), nb::is_flag())
        .value("Normalized", sj::Flag::Normalized, D(Flag, Normalized))
        .value("Gamma",      sj::Flag::Gamma,      D(Flag, Gamma))
        .value("Weight",     sj::Flag::Weight,     D(Flag, Weight))
        .value("Alpha",      sj::Flag::Alpha,      D(Flag, Alpha))
        .value("PremultipliedAlpha", sj::Flag::PremultipliedAlpha, D(Flag, PremultipliedAlpha))
        .value("Check",      sj::Flag::Check,      D(Flag, Check))
        .value("Default",    sj::Flag::Default,    D(Flag, Default));

    nb::class_<sj::Field> field(m, "Field", D(Field));

    field.def(nb::init<>())
         .def(nb::init<const sj::Field &>())
         .def_rw("name", &sj::Field::name, D(Field, name))
         .def_rw("type", &sj::Field::type, D(Field, type))
         .def_rw("offset", &sj::Field::offset, D(Field, offset))
         .def_rw("flags", &sj::Field::flags, D(Field, flags))
         .def_rw("blend", &sj::Field::blend, D(Field, blend))
         .def_prop_rw("value",
             &field_value_to_python,
             &field_value_from_python,
             D(Field, value));

    field.def(nb::self == nb::self)
         .def(nb::self != nb::self)
         .def("__repr__", [](const sj::Field &f) {
             std::ostringstream oss;
             oss << "Field[\n  "<< f << "\n]";
             return oss.str();
         });

    nb::class_<sj::Struct>(m, "Struct", D(Struct))
        .def(nb::init<bool, sj::ByteOrder>(), D(Struct, Struct),
             "pack"_a = false, "byte_order"_a = sj::ByteOrder::Native)
        // Construct a Struct from a NumPy structured dtype (`dt` is a
        // `numpy.dtype` object). `dt` is passed as an untyped `nb::handle`
        // because nanobind has no native caster for NumPy dtypes; the fields,
        // offsets, itemsize, and byte order are read reflectively below.
        .def("__init__", [](sj::Struct *self, nb::handle dt) {
            sj::Struct s = struct_from_dtype(dt);
            new (self) sj::Struct(std::move(s));
        }, "dtype"_a)
        .def(nb::init<const sj::Struct &>())
        .def("append", [](sj::Struct *s, const std::string &name, sj::Type type, uint32_t flags, nb::object value_py) -> sj::Struct* {
            uint64_t value = 0;
            const void *value_ptr = nullptr;
            if (!value_py.is_none()) {
                object_to_value_bits(type, value_py, value);
                value_ptr = &value;
            }
            s->append(name, type, flags, value_ptr);
            return s;
        }, "name"_a, "type"_a, "flags"_a = 0, "value"_a = nb::none(),
           nb::rv_policy::reference, D(Struct, append))
        .def("append", (sj::Struct & (sj::Struct::*)(const sj::Field &)) &sj::Struct::append,
             "field"_a, nb::rv_policy::reference, D(Struct, append, 2))
        .def("align", &sj::Struct::align, D(Struct, align))
        .def("nbytes", &sj::Struct::nbytes, D(Struct, nbytes))
        .def("validate", &sj::Struct::validate,
             "Validate field flags, layout ordering, and shared backend limits")
        .def("__len__", &sj::Struct::size, D(Struct, size))
        .def("pack", &sj::Struct::pack, D(Struct, pack))
        .def("set_pack", &sj::Struct::set_pack, D(Struct, set_pack))
        .def("byte_order", &sj::Struct::byte_order, D(Struct, byte_order))
        .def("set_byte_order", &sj::Struct::set_byte_order, D(Struct, set_byte_order))
        .def("__contains__", &sj::Struct::contains, D(Struct, contains))
        .def("__getitem__", (sj::Field & (sj::Struct::*)(const std::string &)) &sj::Struct::operator[],
             nb::rv_policy::reference_internal)
        .def("__getitem__", [](sj::Struct &s, size_t i) -> sj::Field& {
            if (i >= s.size())
                throw nb::index_error();
            return s[i];
        }, nb::rv_policy::reference_internal)
        .def(nb::self == nb::self)
        .def(nb::self != nb::self)
        .def("__repr__", [](const sj::Struct &f) {
            std::ostringstream oss;
            oss << f;
            return oss.str();
        })
        .def("dtype", [](const sj::Struct &s) -> nb::object {
                nb::list names, offsets, formats;

                char type[4] { };
                type[0] = s.byte_order() == sj::ByteOrder::BigEndian ? '>' : '<';

                for (const sj::Field &field: s) {
                    const sj::TypeInfo &info = sj::type_info(field.type);
                    if (info.numpy_kind == '\0')
                        throw nb::type_error("struct_jit.Struct.dtype(): unsupported type!");

                    type[1] = info.numpy_kind;
                    type[2] = "012345678"[info.size];

                    names.append(nb::str(field.name.c_str()));
                    offsets.append(nb::int_(field.offset));
                    formats.append(nb::str(type));
                }

                nb::dict spec;
                spec["names"] = names;
                spec["formats"] = formats;
                spec["offsets"] = offsets;
                spec["itemsize"] = s.nbytes();

                nb::object np_dtype = nb::module_::import_("numpy").attr("dtype");
                return np_dtype(spec);
            }, "Return an equivalent NumPy dtype")
        ;

    nb::class_<sj::Converter>(m, "Converter", D(Converter))
        .def(nb::init<const sj::Struct &, const sj::Struct &, bool, bool, sj::Type>(),
             "in"_a, "out"_a, "jit"_a = true, "dither"_a = false,
             "working_precision"_a = sj::Type::Float32)
        .def("in", &sj::Converter::in, nb::rv_policy::reference_internal,
             D(Converter, in))
        .def("out", &sj::Converter::out, nb::rv_policy::reference_internal,
             D(Converter, out))
        .def("kernel", [](const sj::Converter &c) -> nb::object {
            auto [ptr, size] = c.kernel();
            if (!ptr)
                return nb::none();
            return nb::bytes((const char *) ptr, size);
        }, D(Converter, kernel))
        .def("convert",
             [](const sj::Converter &c, InputArray input,
                size_t height) -> nb::bytes {
                 return convert_buffer(c, input.data(), input.nbytes(), height);
             }, "input"_a, "height"_a = 1, D(Converter, convert));

    m.def("native_byte_order", &sj::native_byte_order, D(native_byte_order));
    m.def("make_converter", &sj::make_converter,
          "in"_a, "out"_a, "jit"_a = true, "dither"_a = false,
          "working_precision"_a = sj::Type::Float32,
          nb::rv_policy::reference,
          "Return an existing matching converter or create and cache one");
    m.def("clear_cache", &sj::clear_cache,
          "Remove all cached converters and release their executable kernels");
    m.def("type_is_signed_int", &sj::type_is_signed_int, D(type_is_signed_int));
    m.def("type_is_unsigned_int", &sj::type_is_unsigned_int, D(type_is_unsigned_int));
    m.def("type_is_signed", &sj::type_is_signed, D(type_is_signed));
    m.def("type_is_float", &sj::type_is_float, D(type_is_float));
    m.def("type_size", &sj::type_size, D(type_size));
    m.def("type_range", &sj::type_range, D(type_range));
}
