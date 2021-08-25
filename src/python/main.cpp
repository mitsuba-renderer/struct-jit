#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <struct-jit/struct-jit.h>

using namespace pybind11::literals;

namespace py = pybind11;
namespace sj = struct_jit;

#define D(...) ""

PYBIND11_MODULE(struct_jit_ext, m_) {
    (void) m_;
    py::module_ m = py::module::import("struct_jit");
    m.attr("__doc__") = "Struct-JIT";

    py::enum_<sj::Type>(m, "Type")
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
        .value("Invalid", sj::Type::Invalid, D(Type, Invalid))
        .def(py::init([](py::dtype dt) {
            sj::Type value = sj::Type::Int8;
            if (dt.kind() == 'i') {
                switch (dt.itemsize()) {
                    case 1: value = sj::Type::Int8; break;
                    case 2: value = sj::Type::Int16; break;
                    case 4: value = sj::Type::Int32; break;
                    case 8: value = sj::Type::Int64; break;
                    default: throw py::type_error("struct_jit.Type(): Invalid integer type!");
                }
            } else if (dt.kind() == 'u') {
                switch (dt.itemsize()) {
                    case 1: value = sj::Type::UInt8; break;
                    case 2: value = sj::Type::UInt16; break;
                    case 4: value = sj::Type::UInt32; break;
                    case 8: value = sj::Type::UInt64; break;
                    default: throw py::type_error("struct_jit.Type(): Invalid unsigned integer type!");
                }
            } else if (dt.kind() == 'f') {
                switch (dt.itemsize()) {
                    case 2: value = sj::Type::Float16; break;
                    case 4: value = sj::Type::Float32; break;
                    case 8: value = sj::Type::Float64; break;
                    default: throw py::type_error("struct_jit.Type(): Invalid floating point type!");
                }
            } else {
                throw py::type_error("struct_jit.Type(): Invalid input type!");
            }
            return new sj::Type(value);
        }), "dtype"_a);

    py::enum_<sj::Flag>(m, "Flag", py::arithmetic())
        .value("Normalized",         sj::Flag::Normalized, D(Flag, Normalized))
        .value("Gamma",              sj::Flag::Gamma,   D(Flag, Gamma))
        .value("Weight",             sj::Flag::Weight,  D(Flag, Weight))
        .value("Assert",             sj::Flag::Assert,  D(Flag, Assert))
        .value("Alpha",              sj::Flag::Alpha,  D(Flag, Alpha))
        .value("PremultipliedAlpha", sj::Flag::PremultipliedAlpha,  D(Flag, PremultipliedAlpha))
        .value("Default",            sj::Flag::Default, D(Flag, Default))
        .def(py::self | py::self)
        .def(int() | py::self)
        .def(int() & py::self);
}
