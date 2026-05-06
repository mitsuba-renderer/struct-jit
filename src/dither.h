#pragma once

#include <struct-jit/fwd.h>

NAMESPACE_BEGIN(struct_jit)

/// 256x256 dither threshold matrix in [-0.5, 0.5); see dither.cpp.
extern const float dither_matrix256[65536];

NAMESPACE_END(struct_jit)
