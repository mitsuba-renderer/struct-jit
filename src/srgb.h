/*
 * Exact sRGB <-> linear transfer functions, shared by the conversion backends.
 * Gamma fields are always normalized (Struct::validate() enforces it), so these
 * operate on [0, 1] values.
 */
#pragma once

#include <cmath>

NAMESPACE_BEGIN(struct_jit)

/// Decode an sRGB-encoded value to linear.
template <typename T> T srgb_to_linear(T s) {
    if (s <= T(0.04045))
        return s * (T(1) / T(12.92));
    return std::pow((s + T(0.055)) * (T(1) / T(1.055)), T(2.4));
}

/// Encode a linear value as sRGB.
template <typename T> T linear_to_srgb(T l) {
    if (l <= T(0.0031308))
        return l * T(12.92);
    return T(1.055) * std::pow(l, T(1) / T(2.4)) - T(0.055);
}

NAMESPACE_END(struct_jit)
