/*
 * Data structures describing conversion of a scalar field
 *
 * \ref Converter::create_plan() reduces a structure conversion to a list of
 * independent (input field, output field) transfers. \ref make_transfer()
 * resolves one such entry into a \ref Transfer: the two field pointers plus the
 * handful of derived decisions (whether a numeric conversion is needed, in
 * which floating point precision, and whether either side is normalized) that
 * the JIT code generator (jit.cpp) and the software fallback (converter.cpp)
 * both consume. Sharing this one recipe is what keeps the two backends
 * bit-identical.
 */
#pragma once

#include "type_info.h"

NAMESPACE_BEGIN(struct_jit)

/// Register type a value occupies for computation: f16 is widened to f32,
/// everything else keeps its own type.
inline Type register_type(Type type) {
    return type == Type::Float16 ? Type::Float32 : type;
}

/// One resolved scalar transfer, shared verbatim by the JIT and the fallback.
struct Transfer {
    /// Source field to read, or null when missing (substitute the output's
    /// default value).
    const Field *input = nullptr;

    /// Destination field to write (always set for plan entries; the planner
    /// never produces an output-less transfer).
    const Field *output = nullptr;

    /// Read the source as a [0, 1]-normalized integer.
    bool input_normalized = false;

    /// Write the destination as a [0, 1]-normalized integer.
    bool output_normalized = false;

    /// Whether any numeric step is required (only meaningful when both input
    /// and output are present).
    bool needs_conversion = false;

    /// Floating point precision the conversion computes in (valid only when
    /// \ref needs_conversion is set).
    Type working_type = Type::Invalid;

    /// Destination type after f16 -> f32 register widening; the register type
    /// feeding the final store.
    Type output_register_type = Type::Invalid;

    /// Divide this field by the source weight (multiply by the reciprocal).
    bool weight_apply = false;

    /// Premultiply this field by the source alpha (input is straight, output is
    /// premultiplied). Applied after the optional weight division.
    bool alpha_premul = false;

    /// Unpremultiply this field by the source alpha (input is premultiplied,
    /// output is straight). Multiplies by the alpha reciprocal.
    bool alpha_unpremul = false;

    /// Decode the input from sRGB to linear (input is gamma-encoded, output
    /// linear). Applied to the [0, 1] value after input normalization.
    bool gamma_decode = false;

    /// Encode the output from linear to sRGB (output is gamma-encoded, input
    /// linear). Applied to the [0, 1] value before output normalization.
    bool gamma_encode = false;
};

/// Resolve a (input index, output index) plan entry into a \ref Transfer,
/// leaving a field pointer null where the entry holds \ref None. \c working is
/// the converter's single floating point working precision (f32 or f64), used
/// for every numeric step. \c weight_divide forces a float round-trip and a
/// division by the source weight on every mapped field (weighted -> unweighted).
/// \c alpha_apply enables premultiplied-alpha conversion: a mapped field whose
/// input and output disagree on \ref Flag::PremultipliedAlpha is multiplied by
/// the source alpha or its reciprocal. The weight and alpha *source* fields are
/// loaded separately by the backends (see the converter preamble), not here.
inline Transfer make_transfer(const Struct &in, const Struct &out,
                              const std::pair<size_t, size_t> &entry,
                              Type working,
                              bool weight_divide = false,
                              bool alpha_apply = false) {
    Transfer t;
    if (entry.first != None)
        t.input = &in[entry.first];
    if (entry.second != None)
        t.output = &out[entry.second];

    if (t.input)
        t.input_normalized = has_flag(t.input->flags, Flag::Normalized);

    if (t.output) {
        t.output_normalized = has_flag(t.output->flags, Flag::Normalized);
        t.output_register_type = register_type(t.output->type);
    }

    // A numeric conversion is needed whenever the input and output disagree on
    // register type, normalization, or gamma encoding.
    bool input_gamma = false, output_gamma = false;
    if (t.input && t.output) {
        input_gamma  = has_flag(t.input->flags,  Flag::Gamma);
        output_gamma = has_flag(t.output->flags, Flag::Gamma);
        t.needs_conversion =
            register_type(t.input->type) != t.output_register_type ||
            t.input_normalized != t.output_normalized ||
            input_gamma != output_gamma;
        if (t.needs_conversion)
            t.working_type = working;
    }

    if (weight_divide && t.input && t.output) {
        // Force a float round-trip so the value can be scaled by the weight.
        t.weight_apply = true;
        t.needs_conversion = true;
        t.working_type = working;
    }

    // Premultiplied-alpha conversion, skipping the special weight/alpha fields.
    if (alpha_apply && t.input && t.output &&
        !has_flag(t.output->flags, Flag::Weight) &&
        !has_flag(t.output->flags, Flag::Alpha)) {
        bool source_premult = has_flag(t.input->flags,  Flag::PremultipliedAlpha);
        bool target_premult = has_flag(t.output->flags, Flag::PremultipliedAlpha);
        if (target_premult && !source_premult)
            t.alpha_premul = true;
        else if (source_premult && !target_premult)
            t.alpha_unpremul = true;
        if (t.alpha_premul || t.alpha_unpremul) {
            t.needs_conversion = true;
            t.working_type = working;
        }
    }

    // Gamma is handled by a round-trip through linear space: decode the input if
    // it is sRGB-encoded, re-encode the output if it is. This only runs when some
    // numeric conversion already does (so an identical gamma field on both sides
    // is copied untouched, but a gamma field that is rescaled, weighted, or
    // (un)premultiplied is correctly decoded before and re-encoded after the
    // linear-space step).
    if (t.needs_conversion) {
        t.gamma_decode = input_gamma;
        t.gamma_encode = output_gamma;
    }
    return t;
}

NAMESPACE_END(struct_jit)
