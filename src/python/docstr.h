/*
  This file contains docstrings for use in the Python bindings.
  Do not edit! They were automatically extracted by pybind11_mkdoc.
 */

#define __EXPAND(x)                                      x
#define __COUNT(_1, _2, _3, _4, _5, _6, _7, COUNT, ...)  COUNT
#define __VA_SIZE(...)                                   __EXPAND(__COUNT(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1))
#define __CAT1(a, b)                                     a ## b
#define __CAT2(a, b)                                     __CAT1(a, b)
#define __DOC1(n1)                                       __doc_##n1
#define __DOC2(n1, n2)                                   __doc_##n1##_##n2
#define __DOC3(n1, n2, n3)                               __doc_##n1##_##n2##_##n3
#define __DOC4(n1, n2, n3, n4)                           __doc_##n1##_##n2##_##n3##_##n4
#define __DOC5(n1, n2, n3, n4, n5)                       __doc_##n1##_##n2##_##n3##_##n4##_##n5
#define __DOC6(n1, n2, n3, n4, n5, n6)                   __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6
#define __DOC7(n1, n2, n3, n4, n5, n6, n7)               __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6##_##n7
#define DOC(...)                                         __EXPAND(__EXPAND(__CAT2(__DOC, __VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif


static const char *__doc_struct_jit_ByteOrder = R"doc(Byte order of the fields in the ``Struct``)doc";

static const char *__doc_struct_jit_ByteOrder_BigEndian = R"doc()doc";

static const char *__doc_struct_jit_ByteOrder_LittleEndian = R"doc()doc";

static const char *__doc_struct_jit_ByteOrder_Native = R"doc()doc";

static const char *__doc_struct_jit_Field = R"doc(Specifies a single field of a Struct instance)doc";

static const char *__doc_struct_jit_Field_default_value = R"doc(Default value)doc";

static const char *__doc_struct_jit_Field_flags = R"doc(Additional flags)doc";

static const char *__doc_struct_jit_Field_name = R"doc(Name of the field)doc";

static const char *__doc_struct_jit_Field_offset = R"doc(Offset within the ``Struct`` (in bytes))doc";

static const char *__doc_struct_jit_Field_type = R"doc(Type identifier)doc";

static const char *__doc_struct_jit_Flag = R"doc(Optional flags that can be applied to each field)doc";

static const char *__doc_struct_jit_Flag_Check =
R"doc(When converting, check that the field value matches the specified
default value. Otherwise, return a conversion failure.)doc";

static const char *__doc_struct_jit_Flag_Default =
R"doc(When the field is missing in the source record, replace it by the
specified default value.)doc";

static const char *__doc_struct_jit_Flag_Gamma =
R"doc(The field encodes a sRGB gamma-corrected value. Assumes ``Normalized``
is also specified.)doc";

static const char *__doc_struct_jit_Flag_Normalized =
R"doc(The integral field encodes a quantized value in the range [0, 1].
Ignored on fields with a floating point type.)doc";

static const char *__doc_struct_jit_Flag_Weight =
R"doc(The field stores a weight value. All other struct members are
considered relative to this weight. Converting to an un-weighted
structure thus entails a division by the weight. Useful for physically
based rendering, where the weight tracks the accumulated contribution
of Monte Carlo samples.)doc";

static const char *__doc_struct_jit_Struct = R"doc()doc";

static const char *__doc_struct_jit_Struct_Struct =
R"doc(Create an empty data structure

Parameter ``pack``:
    If ``True``, fields will be tightly packed without adding
    alignment-related padding

Parameter ``byte_order``:
    Enables overriding the byte order of the data structure. If
    needed, Struct-JIT will perform endianness conversion during
    conversions.)doc";

static const char *__doc_struct_jit_Struct_Struct_2 = R"doc(Standard constructor/copy assignment/..)doc";

static const char *__doc_struct_jit_Struct_Struct_3 = R"doc()doc";

static const char *__doc_struct_jit_Struct_align = R"doc(Return the alignment (in bytes) of the data structure)doc";

static const char *__doc_struct_jit_Struct_append =
R"doc(Append a new field to the ``Struct``; determines size and offset
automatically)doc";

static const char *__doc_struct_jit_Struct_append_2 = R"doc(Append a new field to the ``Struct`` (manual version))doc";

static const char *__doc_struct_jit_Struct_begin = R"doc(Return an iterator associated with the first field)doc";

static const char *__doc_struct_jit_Struct_begin_2 = R"doc(Return an iterator associated with the first field)doc";

static const char *__doc_struct_jit_Struct_byte_order = R"doc(Return the byte order of the ``Struct``)doc";

static const char *__doc_struct_jit_Struct_end = R"doc(Return an iterator associated with the end of the data structure)doc";

static const char *__doc_struct_jit_Struct_end_2 = R"doc(Return an iterator associated with the end of the data structure)doc";

static const char *__doc_struct_jit_Struct_field = R"doc(Look up a field by name (throws an exception if not found))doc";

static const char *__doc_struct_jit_Struct_field_2 = R"doc(Look up a field by name. Throws an exception if not found)doc";

static const char *__doc_struct_jit_Struct_field_count = R"doc(Return the number of fields)doc";

static const char *__doc_struct_jit_Struct_has_field = R"doc(Check if the ``Struct`` has a field of the specified name)doc";

static const char *__doc_struct_jit_Struct_m_byte_order = R"doc()doc";

static const char *__doc_struct_jit_Struct_m_fields = R"doc()doc";

static const char *__doc_struct_jit_Struct_m_pack = R"doc()doc";

static const char *__doc_struct_jit_Struct_operator_array = R"doc(Access an individual field by index)doc";

static const char *__doc_struct_jit_Struct_operator_array_2 = R"doc(Access an individual field by index)doc";

static const char *__doc_struct_jit_Struct_operator_assign = R"doc()doc";

static const char *__doc_struct_jit_Struct_operator_assign_2 = R"doc()doc";

static const char *__doc_struct_jit_Struct_pack = R"doc(Return whether or not the ``Struct`` is packed)doc";

static const char *__doc_struct_jit_Struct_size = R"doc(Return the size (in bytes) of the data structure, including padding)doc";

static const char *__doc_struct_jit_Type = R"doc(List of field types supported by Struct-JIT)doc";

static const char *__doc_struct_jit_Type_Float16 = R"doc()doc";

static const char *__doc_struct_jit_Type_Float32 = R"doc()doc";

static const char *__doc_struct_jit_Type_Float64 = R"doc()doc";

static const char *__doc_struct_jit_Type_Int16 = R"doc()doc";

static const char *__doc_struct_jit_Type_Int32 = R"doc()doc";

static const char *__doc_struct_jit_Type_Int64 = R"doc()doc";

static const char *__doc_struct_jit_Type_Int8 = R"doc()doc";

static const char *__doc_struct_jit_Type_Invalid = R"doc()doc";

static const char *__doc_struct_jit_Type_UInt16 = R"doc()doc";

static const char *__doc_struct_jit_Type_UInt32 = R"doc()doc";

static const char *__doc_struct_jit_Type_UInt64 = R"doc()doc";

static const char *__doc_struct_jit_Type_UInt8 = R"doc()doc";

static const char *__doc_struct_jit_has_flag = R"doc()doc";

static const char *__doc_struct_jit_is_float = R"doc(Check whether the given type is a floating point type)doc";

static const char *__doc_struct_jit_is_signed_int = R"doc(Check whether the given type is a signed integer)doc";

static const char *__doc_struct_jit_is_unsigned_int = R"doc(Check whether the given type is an unsigned integer)doc";

static const char *__doc_struct_jit_operator_add = R"doc()doc";

static const char *__doc_struct_jit_operator_band = R"doc()doc";

static const char *__doc_struct_jit_operator_band_2 = R"doc()doc";

static const char *__doc_struct_jit_operator_bnot = R"doc()doc";

static const char *__doc_struct_jit_operator_bor = R"doc()doc";

static const char *__doc_struct_jit_operator_bor_2 = R"doc()doc";

static const char *__doc_struct_jit_operator_eq = R"doc()doc";

static const char *__doc_struct_jit_operator_eq_2 = R"doc()doc";

static const char *__doc_struct_jit_operator_lshift = R"doc()doc";

static const char *__doc_struct_jit_operator_lshift_2 = R"doc()doc";

static const char *__doc_struct_jit_operator_lshift_3 = R"doc()doc";

static const char *__doc_struct_jit_operator_lshift_4 = R"doc()doc";

static const char *__doc_struct_jit_operator_lshift_5 = R"doc()doc";

static const char *__doc_struct_jit_operator_ne = R"doc()doc";

static const char *__doc_struct_jit_operator_ne_2 = R"doc()doc";

static const char *__doc_struct_jit_size = R"doc(Return the size in bytes of the given variable type)doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

