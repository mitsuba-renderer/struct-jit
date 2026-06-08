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

static const char *__doc_struct_jit_Converter =
R"doc(Converts records from one Struct layout to another.

Given an input and output Struct, the ``Converter`` analyzes the two
layouts and builds a plan that maps each output field to a matching
input field (or a default value). When ``jit`` is enabled, it compiles
this plan into a native machine-code kernel; otherwise a portable
software fallback performs the same conversion. A single ``Converter``
can be reused to convert many elements (e.g. a 2D image) in one
convert() call.)doc";

static const char *__doc_struct_jit_Converter_BlendEntry =
R"doc(One blended output field: its index plus the (input index, weight)
terms summed into it.)doc";

static const char *__doc_struct_jit_Converter_BlendEntry_output = R"doc()doc";

static const char *__doc_struct_jit_Converter_BlendEntry_terms = R"doc()doc";

static const char *__doc_struct_jit_Converter_Converter =
R"doc(Parameter ``working_precision``:
    Floating point precision used for the numeric conversion steps. A
    single working type is used for all fields; must be
    ``Type::Float32`` (the default) or ``Type::Float64``.)doc";

static const char *__doc_struct_jit_Converter_Converter_2 = R"doc()doc";

static const char *__doc_struct_jit_Converter_Converter_3 = R"doc()doc";

static const char *__doc_struct_jit_Converter_convert = R"doc()doc";

static const char *__doc_struct_jit_Converter_convert_fallback =
R"doc(Convert a 2D region of records using the portable software
interpreter.)doc";

static const char *__doc_struct_jit_Converter_convert_fallback_impl = R"doc()doc";

static const char *__doc_struct_jit_Converter_create_kernel = R"doc(Compile m_plan into a native machine-code kernel (m_kernel).)doc";

static const char *__doc_struct_jit_Converter_create_plan =
R"doc(Analyze the input/output layouts and build m_plan, the per-field
transfer schedule shared by the JIT and software fallback.)doc";

static const char *__doc_struct_jit_Converter_source = R"doc()doc";

static const char *__doc_struct_jit_Converter_kernel =
R"doc(Return the generated kernel machine code (e.g. for disassembly), or
{nullptr, 0} when no kernel was compiled)doc";

static const char *__doc_struct_jit_Converter_m_alpha_apply =
R"doc(Both sides declare Flag::Alpha, so fields can be (un)premultiplied by
m_alpha_in.)doc";

static const char *__doc_struct_jit_Converter_m_alpha_in = R"doc(Source alpha field index for m_alpha_apply (``size_t``(-1) if unused).)doc";

static const char *__doc_struct_jit_Converter_m_blend =
R"doc(Output fields computed as a linear combination of inputs
(Field::blend); applied after m_plan.)doc";

static const char *__doc_struct_jit_Converter_m_dither = R"doc(Apply ordered dithering when quantizing float -> int (2D images).)doc";

static const char *__doc_struct_jit_Converter_m_in = R"doc()doc";

static const char *__doc_struct_jit_Converter_m_kernel = R"doc(Compiled kernel and its size in bytes (null when using the fallback).)doc";

static const char *__doc_struct_jit_Converter_m_kernel_size = R"doc()doc";

static const char *__doc_struct_jit_Converter_m_out = R"doc()doc";

static const char *__doc_struct_jit_Converter_m_plan =
R"doc(Ordered transfer schedule produced by create_plan(). Each entry is a
(input field index, output field index) pair; None in the input slot
marks a missing input (substitute the output default).)doc";

static const char *__doc_struct_jit_Converter_m_weight_divide =
R"doc(Weighted -> unweighted: divide every mapped field by the source weight
m_weight_in.)doc";

static const char *__doc_struct_jit_Converter_m_weight_in =
R"doc(Source weight field index for m_weight_divide (``size_t``(-1) if
unused).)doc";

static const char *__doc_struct_jit_Converter_m_working =
R"doc(Working precision (Float32 or Float64) used for every numeric
conversion step.)doc";

static const char *__doc_struct_jit_Converter_operator_assign = R"doc()doc";

static const char *__doc_struct_jit_Converter_operator_assign_2 = R"doc()doc";

static const char *__doc_struct_jit_Converter_target = R"doc()doc";

static const char *__doc_struct_jit_Converter_release_kernel = R"doc(Release the executable memory backing m_kernel.)doc";

static const char *__doc_struct_jit_Field = R"doc(Specifies a single field of a Struct instance)doc";

static const char *__doc_struct_jit_Field_blend =
R"doc(Blend specification: when non-empty, the field's value is a linear
combination of other (named) input fields rather than a direct copy.

Each entry is a \c (weight, source field name) pair; the output is
``sum(weight_i * linearize(source_i))``, evaluated in the converter's
working precision.)doc";

static const char *__doc_struct_jit_Field_flags = R"doc(Bitwise combination of Flag values)doc";

static const char *__doc_struct_jit_Field_name = R"doc(Name of the field)doc";

static const char *__doc_struct_jit_Field_offset = R"doc(Offset within the ``Struct`` (in bytes))doc";

static const char *__doc_struct_jit_Field_type = R"doc()doc";

static const char *__doc_struct_jit_Field_value = R"doc(Default value (reinterpreted according to type))doc";

static const char *__doc_struct_jit_Flag = R"doc(Optional flags that can be applied to each field)doc";

static const char *__doc_struct_jit_Flag_Alpha =
R"doc(The field stores an alpha value. When both the input and output
declare an alpha field, all other fields can be converted between
premultiplied and non-premultiplied representations (see
``PremultipliedAlpha``).)doc";

static const char *__doc_struct_jit_Flag_Check =
R"doc(When converting, check that the field value matches the specified
default value. Otherwise, return a conversion failure.)doc";

static const char *__doc_struct_jit_Flag_Default =
R"doc(When the field is missing in the source record, replace it by the
specified default value.)doc";

static const char *__doc_struct_jit_Flag_Gamma =
R"doc(The field encodes a sRGB gamma-corrected value. Requires that
``Normalized`` is also specified.)doc";

static const char *__doc_struct_jit_Flag_Normalized =
R"doc(The integral field encodes a quantized value in the range [0, 1]. An
error is raised if this flag is specified for a floating point-typed
field.)doc";

static const char *__doc_struct_jit_Flag_PremultipliedAlpha =
R"doc(The field is premultiplied by the value of the ``Alpha`` field.
Converting between an input and output that disagree on this flag
multiplies the field by the alpha (premultiply) or its reciprocal
(unpremultiply).)doc";

static const char *__doc_struct_jit_Flag_Weight =
R"doc(The field stores a weight value. All other struct members are
considered relative to this weight. Converting to an un-weighted
structure thus entails a division by the weight. Useful for physically
based rendering, where the weight tracks the accumulated contribution
of Monte Carlo samples.)doc";

static const char *__doc_struct_jit_Struct =
R"doc(Describes the in-memory layout of a record as an ordered list of
named, typed Field entries.

A ``Struct`` is the schema that a Converter reads from and writes to.
It tracks each field's type, byte offset, optional Flag annotations,
and default value, along with structure-wide properties such as the
byte order and whether fields are tightly packed or padded for natural
alignment.)doc";

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

static const char *__doc_struct_jit_Struct_append = R"doc(Append a new field, while determining size and offset automatically)doc";

static const char *__doc_struct_jit_Struct_append_2 =
R"doc(Append a new field to the ``Struct`` (all information must be
provided))doc";

static const char *__doc_struct_jit_Struct_begin = R"doc(Return an iterator associated with the first field)doc";

static const char *__doc_struct_jit_Struct_begin_2 = R"doc(Return an iterator associated with the first field)doc";

static const char *__doc_struct_jit_Struct_byte_order = R"doc(Return the byte order of the ``Struct``)doc";

static const char *__doc_struct_jit_Struct_contains = R"doc(Check if the ``Struct`` has a field of the specified name)doc";

static const char *__doc_struct_jit_Struct_end =
R"doc(Return an iterator associated with the end of the data structure
[const])doc";

static const char *__doc_struct_jit_Struct_end_2 = R"doc(Return an iterator associated with the end of the data structure)doc";

static const char *__doc_struct_jit_Struct_find =
R"doc(Return an iterator that points to a field with the specified name
[const])doc";

static const char *__doc_struct_jit_Struct_find_2 = R"doc(Return an iterator that points to a field with the specified name)doc";

static const char *__doc_struct_jit_Struct_m_byte_order = R"doc()doc";

static const char *__doc_struct_jit_Struct_m_fields = R"doc()doc";

static const char *__doc_struct_jit_Struct_m_pack = R"doc()doc";

static const char *__doc_struct_jit_Struct_nbytes =
R"doc(Return the total size (in bytes) of the data structure, including
padding)doc";

static const char *__doc_struct_jit_Struct_operator_array = R"doc(Access an individual field by index [const])doc";

static const char *__doc_struct_jit_Struct_operator_array_2 = R"doc(Access an individual field by index)doc";

static const char *__doc_struct_jit_Struct_operator_array_3 = R"doc(Access an individual field by name [const])doc";

static const char *__doc_struct_jit_Struct_operator_array_4 = R"doc(Access an individual field by name)doc";

static const char *__doc_struct_jit_Struct_operator_assign = R"doc()doc";

static const char *__doc_struct_jit_Struct_operator_assign_2 = R"doc()doc";

static const char *__doc_struct_jit_Struct_pack = R"doc(Are appended fields tightly packed (i.e. without alignment padding)?)doc";

static const char *__doc_struct_jit_Struct_set_byte_order = R"doc()doc";

static const char *__doc_struct_jit_Struct_set_pack = R"doc()doc";

static const char *__doc_struct_jit_Struct_size = R"doc(Return the number of fields)doc";

static const char *__doc_struct_jit_Struct_validate = R"doc(Validate field definitions and shared backend layout limits)doc";

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

static const char *__doc_struct_jit_clear_cache = R"doc(Remove all cached converters and release their executable kernels.)doc";

static const char *__doc_struct_jit_has_flag = R"doc()doc";

static const char *__doc_struct_jit_make_converter = R"doc(Return an existing matching converter or create and cache one.)doc";

static const char *__doc_struct_jit_native_byte_order = R"doc()doc";

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

static const char *__doc_struct_jit_type_is_float = R"doc(Check whether the given type is a floating point type)doc";

static const char *__doc_struct_jit_type_is_signed = R"doc(Check whether the given type is a signed type)doc";

static const char *__doc_struct_jit_type_is_signed_int = R"doc(Check whether the given type is a signed integer)doc";

static const char *__doc_struct_jit_type_is_unsigned_int = R"doc(Check whether the given type is an unsigned integer)doc";

static const char *__doc_struct_jit_type_range = R"doc(Return the representable range of a particular type)doc";

static const char *__doc_struct_jit_type_size = R"doc(Return the size in bytes of the given variable type)doc";

#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

