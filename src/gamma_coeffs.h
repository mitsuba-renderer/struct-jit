/*
 * sRGB gamma rational-polynomial coefficients, shared by the JIT code generator
 * and (transitively) the snippet layout. Each array is a flat constant table in
 * the exact byte order the gamma_* asm snippets address by offset:
 *
 *     [ linear_scale, threshold, num[0..k], den[0..k] ]
 *
 * The snippet loads element i from <table base> + i * sizeof(element), so these
 * arrays must stay in lock-step with the offsets in asm/asm_aarch64.S and
 * asm/asm_x64.S. The values come from a minimax polynomial fit of the sRGB gamma
 * curve (rel.err ~2e-7 single, ~1.5e-15 double). The codegen appends the chosen
 * array to the deduplicated constant pool and points the snippet's base load at it.
 */
#pragma once

namespace struct_jit {

// sRGB -> linear, single precision (y = x, 5-term num/den).
inline constexpr float gamma_decode_f4_coeffs[] = {
    0.07739938080495357f,  // 1/12.92
    0.04045f,              // threshold
    // num[0..4]
    -36.04572663838034f, -47.46726633009393f, -11.199318357635072f,
    -0.7386328024653209f, -0.0163933279112946f,
    // den[0..4]
    1.0f, -18.225745396846637f, -59.096406619244426f,
    -19.140923959601675f, -0.004261480793199332f
};

// linear -> sRGB, single precision (y = sqrt(x), 6-term num/den).
inline constexpr float gamma_encode_f4_coeffs[] = {
    12.92f,                // linear scale
    0.0031308f,            // threshold
    // num[0..5]
    -0.016202083165206348f, 0.7551545191665577f, 2.0041169284241644f,
    0.7642611304733891f, 0.03453868659826638f, -0.0016829072605308378f,
    // den[0..5]
    1.0f, 1.8970238036421054f, 0.6085338522168684f,
    0.03467195408529984f, -0.00004375359692957097f, 4.178892964897981e-7f
};

// sRGB -> linear, double precision (y = x, 10-term num/den).
inline constexpr double gamma_decode_f8_coeffs[] = {
    0.07739938080495357,   // 1/12.92
    0.04045,               // threshold
    // num[0..9]
    -342.62884098034357, -3483.4445569178347, -9735.250875334352,
    -10782.158977031822, -5548.704065887224, -1446.951694673217,
    -200.19589605282445, -14.786385491859248, -0.5489744177844188,
    -0.008042950896814532,
    // den[0..9]
    1.0, -84.8098437770271, -1884.7738197074218, -8059.219012060384,
    -11916.470977597566, -7349.477378676199, -2013.8039726540235,
    -237.47722999429413, -9.646075249097724, -2.2132610916769585e-8
};

// linear -> sRGB, double precision (y = sqrt(x), 11-term num/den).
inline constexpr double gamma_encode_f8_coeffs[] = {
    12.92,                 // linear scale
    0.0031308,             // threshold
    // num[0..10]
    -0.0031151377052754843, 0.5838023820686707, 8.450947414259522,
    27.901125077137042, 32.44669922192121, 15.374469584296442,
    3.0477578489880823, 0.2263810267005674, 0.002531335520959116,
    -0.00021805827098915798, -3.7113872202050023e-6,
    // den[0..10]
    1.0, 10.723011300050162, 29.70548706952188, 30.50364355650628,
    13.297981743005433, 2.575446652731678, 0.21749170309546628,
    0.007244514696840552, 0.00007045228641004039, -8.387527630781522e-9,
    2.2380622409188757e-11
};

} // namespace struct_jit
