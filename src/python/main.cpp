#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>
#include <struct-jit/struct-jit.h>
#include <sstream>

using namespace pybind11::literals;

namespace py = pybind11;
namespace sj = struct_jit;

#define D(...) ""

sj::Type type_from_dtype(const py::dtype &dt) {
    if (dt.kind() == 'i') {
        switch (dt.itemsize()) {
            case 1: return sj::Type::Int8;
            case 2: return sj::Type::Int16;
            case 4: return sj::Type::Int32;
            case 8: return sj::Type::Int64;
        }
    } else if (dt.kind() == 'u') {
        switch (dt.itemsize()) {
            case 1: return sj::Type::UInt8;
            case 2: return sj::Type::UInt16;
            case 4: return sj::Type::UInt32;
            case 8: return sj::Type::UInt64;
        }
    } else if (dt.kind() == 'f') {
        switch (dt.itemsize()) {
            case 2: return sj::Type::Float16;
            case 4: return sj::Type::Float32;
            case 8: return sj::Type::Float64;
        }
    }
    throw py::type_error("struct_jit.Type(): Invalid input type!");
}

PYBIND11_MODULE(struct_jit_ext, m_) {
    (void) m_;
    py::module_ m = py::module::import("struct_jit");
    m.attr("__doc__") = "Struct-JIT";

    py::enum_<sj::ByteOrder>(m, "ByteOrder", D(ByteOrder))
        .value("Native", sj::ByteOrder::Native, D(ByteOrder, Native))
        .value("LittleEndian", sj::ByteOrder::LittleEndian, D(ByteOrder, LittleEndian))
        .value("BigEndian", sj::ByteOrder::BigEndian, D(ByteOrder, BigEndian));

    py::enum_<sj::Type>(m, "Type", D(Type))
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
        .value("Float64", sj::Type::Float64, D(Type, Float64))
        .def(py::init([](const py::dtype &dt) { return type_from_dtype(dt); }));

    m.def("is_float", &sj::is_float, D(is_float))
     .def("is_signed_int", &sj::is_signed_int, D(is_signed_int))
     .def("is_unsigned_int", &sj::is_unsigned_int, D(is_unsigned_int))
     .def("size", &sj::size, D(size));

    py::enum_<sj::Flag>(m, "Flag", py::arithmetic())
        .value("Normalized", sj::Flag::Normalized, D(Flag, Normalized))
        .value("Gamma",      sj::Flag::Gamma,      D(Flag, Gamma))
        .value("Weight",     sj::Flag::Weight,     D(Flag, Weight))
        .value("Check",      sj::Flag::Check,      D(Flag, Check))
        .value("Default",    sj::Flag::Default,    D(Flag, Default))
        .def(py::self | py::self)
        .def(int() | py::self)
        .def(int() & py::self);

    py::class_<sj::Field>(m, "Field", D(Field))
        .def(py::init<>())
        .def(py::init<const sj::Field &>())
        .def_readwrite("name", &sj::Field::name, D(Field, name))
        .def_readwrite("type", &sj::Field::type, D(Field, type))
        .def_readwrite("offset", &sj::Field::offset, D(Field, offset))
        .def_readwrite("flags", &sj::Field::flags, D(Field, flags))
        .def_readwrite("default_value", &sj::Field::default_value, D(Field, default_value))
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const sj::Field &f) {
            std::ostringstream oss;
            oss << "Field[\n  "<< f << "\n]";
            return oss.str();
        });

    py::class_<sj::Struct>(m, "Struct", D(Struct))
        .def(py::init<bool, sj::ByteOrder>(), D(Struct, Struct),
             "pack"_a = false, "byte_order"_a = sj::ByteOrder::Native)
        .def(py::init([](py::dtype dt) {
            py::object fields = dt.attr("fields");

            if (fields.is_none())
                throw py::type_error("struct_jit.Struct(): input is not a structured dtype!");

            sj::Struct s;
            for (py::handle key: fields) {
                py::tuple value = fields[key];
                sj::Field f;
                f.name = py::cast<std::string>(key);
                f.type = type_from_dtype(value[0]);
                f.offset = py::cast<uint32_t>(value[1]);
                s.append(f);
            }

            return s;
        }), "dtype"_a)
        .def(py::init<const sj::Struct &>())
        .def("append", (sj::Struct & (sj::Struct::*)(const std::string &, sj::Type, uint32_t, double)) &sj::Struct::append,
             "name"_a, "type"_a, "flags"_a = 0,
             "default_value"_a = 0.0, D(Struct, append))
        .def("append", (sj::Struct & (sj::Struct::*)(const sj::Field &)) &sj::Struct::append,
             "field"_a, D(Struct, append, 2))
        .def("align", &sj::Struct::align, D(Struct, align))
        .def("size", &sj::Struct::size, D(Struct, size))
        .def("byte_order", &sj::Struct::byte_order, D(Struct, byte_order))
        .def("pack", &sj::Struct::pack, D(Struct, pack))
        .def("has_field", &sj::Struct::has_field, D(Struct, has_field))
        .def("field_count", &sj::Struct::field_count, D(Struct, field_count))
        .def("field", (sj::Field & (sj::Struct::*)(const std::string &)) &sj::Struct::field, D(Struct, field))
        .def("__getitem__", [](sj::Struct &s, size_t i) -> sj::Field& {
            if (i >= s.field_count())
                throw py::index_error();
            return s[i];
        }, py::return_value_policy::reference_internal)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__repr__", [](const sj::Struct &f) {
            std::ostringstream oss;
            oss << f;
            return oss.str();
        })
        .def("dtype", [](const sj::Struct &s) -> py::dtype {
                py::list names, offsets, formats;

                char type[4] { };
                type[0] = s.byte_order() == sj::ByteOrder::BigEndian ? '>' : '<';

                for (const sj::Field &field: s) {
                    if (sj::is_signed_int(field.type))
                        type[1] = 'i';
                    else if (sj::is_unsigned_int(field.type))
                        type[1] = 'u';
                    else if (sj::is_float(field.type))
                        type[1] = 'f';
                    else
                        throw py::type_error("struct_jit.Struct.dtype(): unsupported type!");

                    type[2] = "012345678"[sj::size(field.type)];

                    names.append(py::str(field.name));
                    offsets.append(py::int_(field.offset));
                    formats.append(py::str(type));
                }

                return py::dtype(names, formats, offsets, s.size());
            }, "Return an equivalent NumPy dtype")
        ;
}
