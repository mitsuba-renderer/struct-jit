// AArch64 (ARMv8-A) snippet library for Struct-JIT.
//
// Calling convention (AAPCS): bool convert(const void *in, void *out,
//                                          size_t width, size_t height)
//   x0 = in     x1 = out     x2 = width   x3 = height     w0 = return
//
// Register roles that persist across snippets (must not be clobbered while
// live). Everything else (x7, x9, x10, v1, v3-v7) is per-snippet scratch.
//   x0 in   x1 out   x2 width   x3 height    arguments (w0 = bool return)
//   x4 y    x5 x                             loop counters
//   x6      integer value in flight
//   v0      SIMD value in flight
//   v2      weight reciprocal, live across the whole loop body (weight division)
//
// The kernel is a leaf function using only AAPCS caller-saved registers, so it
// preserves nothing for its caller.
//
// Patch sites:
//   imm12 placeholders use the value 0xFFF (visible in disassembly as
//                                            #0xFFF / #0x1FFE / #0x3FFC / #0x7FF8
//                                            depending on the load/store size).
//   imm9 unscaled placeholders use the byte offset 0xFF.
//   imm19 placeholders use 0x7FFFF, encoded directly via .inst because the
//                                            assembler won't let us write a
//                                            forward branch that far.

#if defined(__APPLE__)
#  define SYM(x) _##x
#else
#  define SYM(x) x
#endif

// `;` is a line comment on Apple's AArch64 assembler, so we can't use it as a
// statement separator inside a macro. Instead, fold the .globl directive into
// the label by exploiting "label: directive" — the label gets defined and
// declared global by a single source line.

#define SNIPPET(name)        SYM(name): .globl SYM(name)
#define SNIPPET_END(name)    SYM(name##_end): .globl SYM(name##_end)

    .text
    .balign 4

// --------------------------------------------------------------------------
// Loop scaffolding
// --------------------------------------------------------------------------

SNIPPET(main_prefix)
    mov     x4, #0                  // y = 0
SNIPPET_END(main_prefix)

SNIPPET(main_start)
    mov     x5, #0                  // x = 0
SNIPPET_END(main_start)

SNIPPET(main_inc_in)
    add     x0, x0, #0xFFF          // in += stride  (imm12 patch)
SNIPPET_END(main_inc_in)

SNIPPET(main_inc_out)
    add     x1, x1, #0xFFF          // out += stride (imm12 patch)
SNIPPET_END(main_inc_out)

SNIPPET(main_suffix_inner)
    add     x5, x5, #1              // x++
    cmp     x5, x2                  // x == width?
    .inst   0x54FFFFE1              // b.ne  (imm19 patch, sentinel = 0x7FFFF)
SNIPPET_END(main_suffix_inner)

SNIPPET(main_suffix_outer)
    add     x4, x4, #1              // y++
    cmp     x4, x3                  // y == height?
    .inst   0x54FFFFE1              // b.ne  (imm19 patch, sentinel = 0x7FFFF)
SNIPPET_END(main_suffix_outer)

SNIPPET(main_postfix)
    mov     w0, #1                  // return true
    ret
SNIPPET_END(main_postfix)

// Failure epilogue: a 'Check'-flagged field did not match its expected value.
// Return false. Branched to from chk_ne (forward b.ne, patched by the codegen).
SNIPPET(fail_postfix)
    mov     w0, #0                  // return false
    ret
SNIPPET_END(fail_postfix)

// --------------------------------------------------------------------------
// Loads (zero-extending into w6/x6 for ints, into low bits of v0 for FP)
// --------------------------------------------------------------------------

SNIPPET(load_u1)
    ldrb    w6, [x0, #0xFFF]
SNIPPET_END(load_u1)

SNIPPET(load_u2)
    ldrh    w6, [x0, #(0xFFF * 2)]
SNIPPET_END(load_u2)

SNIPPET(load_u4)
    ldr     w6, [x0, #(0xFFF * 4)]
SNIPPET_END(load_u4)

SNIPPET(load_u8)
    ldr     x6, [x0, #(0xFFF * 8)]
SNIPPET_END(load_u8)

SNIPPET(load_u2_unaligned)
    ldurh   w6, [x0, #0xFF]
SNIPPET_END(load_u2_unaligned)

SNIPPET(load_u4_unaligned)
    ldur    w6, [x0, #0xFF]
SNIPPET_END(load_u4_unaligned)

SNIPPET(load_u8_unaligned)
    ldur    x6, [x0, #0xFF]
SNIPPET_END(load_u8_unaligned)

SNIPPET(load_f2)
    ldr     h0, [x0, #(0xFFF * 2)]
    fcvt    s0, h0
SNIPPET_END(load_f2)

SNIPPET(load_f4)
    ldr     s0, [x0, #(0xFFF * 4)]
SNIPPET_END(load_f4)

SNIPPET(load_f8)
    ldr     d0, [x0, #(0xFFF * 8)]
SNIPPET_END(load_f8)

SNIPPET(load_f2_unaligned)
    ldur    h0, [x0, #0xFF]
    fcvt    s0, h0
SNIPPET_END(load_f2_unaligned)

SNIPPET(load_f4_unaligned)
    ldur    s0, [x0, #0xFF]
SNIPPET_END(load_f4_unaligned)

SNIPPET(load_f8_unaligned)
    ldur    d0, [x0, #0xFF]
SNIPPET_END(load_f8_unaligned)

// --------------------------------------------------------------------------
// Stores
// --------------------------------------------------------------------------

SNIPPET(store_u1)
    strb    w6, [x1, #0xFFF]
SNIPPET_END(store_u1)

SNIPPET(store_u2)
    strh    w6, [x1, #(0xFFF * 2)]
SNIPPET_END(store_u2)

SNIPPET(store_u4)
    str     w6, [x1, #(0xFFF * 4)]
SNIPPET_END(store_u4)

SNIPPET(store_u8)
    str     x6, [x1, #(0xFFF * 8)]
SNIPPET_END(store_u8)

SNIPPET(store_u2_unaligned)
    sturh   w6, [x1, #0xFF]
SNIPPET_END(store_u2_unaligned)

SNIPPET(store_u4_unaligned)
    stur    w6, [x1, #0xFF]
SNIPPET_END(store_u4_unaligned)

SNIPPET(store_u8_unaligned)
    stur    x6, [x1, #0xFF]
SNIPPET_END(store_u8_unaligned)

SNIPPET(store_f2)
    fcvt    h0, s0
    str     h0, [x1, #(0xFFF * 2)]
SNIPPET_END(store_f2)

SNIPPET(store_f4)
    str     s0, [x1, #(0xFFF * 4)]
SNIPPET_END(store_f4)

SNIPPET(store_f8)
    str     d0, [x1, #(0xFFF * 8)]
SNIPPET_END(store_f8)

SNIPPET(store_f2_unaligned)
    fcvt    h0, s0
    stur    h0, [x1, #0xFF]
SNIPPET_END(store_f2_unaligned)

SNIPPET(store_f4_unaligned)
    stur    s0, [x1, #0xFF]
SNIPPET_END(store_f4_unaligned)

SNIPPET(store_f8_unaligned)
    stur    d0, [x1, #0xFF]
SNIPPET_END(store_f8_unaligned)

// --------------------------------------------------------------------------
// Endianness swap
// --------------------------------------------------------------------------

SNIPPET(bswap_u2)
    rev16   w6, w6
SNIPPET_END(bswap_u2)

SNIPPET(bswap_u4)
    rev     w6, w6
SNIPPET_END(bswap_u4)

SNIPPET(bswap_u8)
    rev     x6, x6
SNIPPET_END(bswap_u8)

// --------------------------------------------------------------------------
// Bit-cast / format swap between GPR and SIMD
// --------------------------------------------------------------------------

SNIPPET(mov_u2_f2)
    fmov    s0, w6                  // copy bits w6 -> s0 (h0 is low 16)
    fcvt    s0, h0                  // f16 -> f32
SNIPPET_END(mov_u2_f2)

SNIPPET(mov_f2_u2)
    fcvt    h0, s0                  // f32 -> f16
    fmov    w6, s0                  // copy low 32 bits of s0 -> w6
SNIPPET_END(mov_f2_u2)

SNIPPET(mov_u4_f4)
    fmov    s0, w6                  // u32 bit pattern -> s0
SNIPPET_END(mov_u4_f4)

SNIPPET(mov_f4_u4)
    fmov    w6, s0                  // f32 bit pattern -> w6
SNIPPET_END(mov_f4_u4)

SNIPPET(mov_u8_f8)
    fmov    d0, x6
SNIPPET_END(mov_u8_f8)

SNIPPET(mov_f8_u8)
    fmov    x6, d0
SNIPPET_END(mov_f8_u8)

// --------------------------------------------------------------------------
// Integer -> float conversion
//
// w6 holds the input. Loads are zero-extending, so we re-sign-extend for
// signed source types before scvtf.
// --------------------------------------------------------------------------

SNIPPET(cvt_u1_f4)
    ucvtf   s0, w6
SNIPPET_END(cvt_u1_f4)

SNIPPET(cvt_i1_f4)
    sxtb    w6, w6
    scvtf   s0, w6
SNIPPET_END(cvt_i1_f4)

SNIPPET(cvt_u2_f4)
    ucvtf   s0, w6
SNIPPET_END(cvt_u2_f4)

SNIPPET(cvt_i2_f4)
    sxth    w6, w6
    scvtf   s0, w6
SNIPPET_END(cvt_i2_f4)

SNIPPET(cvt_u4_f4)
    ucvtf   s0, w6
SNIPPET_END(cvt_u4_f4)

SNIPPET(cvt_i4_f4)
    scvtf   s0, w6
SNIPPET_END(cvt_i4_f4)

SNIPPET(cvt_u8_f4)
    ucvtf   s0, x6
SNIPPET_END(cvt_u8_f4)

SNIPPET(cvt_i8_f4)
    scvtf   s0, x6
SNIPPET_END(cvt_i8_f4)

SNIPPET(cvt_u1_f8)
    ucvtf   d0, w6
SNIPPET_END(cvt_u1_f8)

SNIPPET(cvt_i1_f8)
    sxtb    w6, w6
    scvtf   d0, w6
SNIPPET_END(cvt_i1_f8)

SNIPPET(cvt_u2_f8)
    ucvtf   d0, w6
SNIPPET_END(cvt_u2_f8)

SNIPPET(cvt_i2_f8)
    sxth    w6, w6
    scvtf   d0, w6
SNIPPET_END(cvt_i2_f8)

SNIPPET(cvt_u4_f8)
    ucvtf   d0, w6
SNIPPET_END(cvt_u4_f8)

SNIPPET(cvt_i4_f8)
    scvtf   d0, w6
SNIPPET_END(cvt_i4_f8)

SNIPPET(cvt_u8_f8)
    ucvtf   d0, x6
SNIPPET_END(cvt_u8_f8)

SNIPPET(cvt_i8_f8)
    scvtf   d0, x6
SNIPPET_END(cvt_i8_f8)

// --------------------------------------------------------------------------
// Float-to-float precision conversion
// --------------------------------------------------------------------------

SNIPPET(cvt_f4_f8)
    fcvt    d0, s0
SNIPPET_END(cvt_f4_f8)

SNIPPET(cvt_f8_f4)
    fcvt    s0, d0
SNIPPET_END(cvt_f8_f4)

// --------------------------------------------------------------------------
// Float-to-integer (truncating, after upstream rounding)
//
// We share four converters per source size: signed/unsigned x w-reg/x-reg.
// The matching store snippet (strb/strh/str w / str x) decides how many
// bytes are actually committed.
// --------------------------------------------------------------------------

SNIPPET(cvt_f4_iw)
    fcvtzs  w6, s0
SNIPPET_END(cvt_f4_iw)

SNIPPET(cvt_f4_uw)
    fcvtzu  w6, s0
SNIPPET_END(cvt_f4_uw)

SNIPPET(cvt_f4_ix)
    fcvtzs  x6, s0
SNIPPET_END(cvt_f4_ix)

SNIPPET(cvt_f4_ux)
    fcvtzu  x6, s0
SNIPPET_END(cvt_f4_ux)

SNIPPET(cvt_f8_iw)
    fcvtzs  w6, d0
SNIPPET_END(cvt_f8_iw)

SNIPPET(cvt_f8_uw)
    fcvtzu  w6, d0
SNIPPET_END(cvt_f8_uw)

SNIPPET(cvt_f8_ix)
    fcvtzs  x6, d0
SNIPPET_END(cvt_f8_ix)

SNIPPET(cvt_f8_ux)
    fcvtzu  x6, d0
SNIPPET_END(cvt_f8_ux)

// --------------------------------------------------------------------------
// Round to nearest, ties-to-even
// --------------------------------------------------------------------------

SNIPPET(round_f4)
    frintn  s0, s0
SNIPPET_END(round_f4)

SNIPPET(round_f8)
    frintn  d0, d0
SNIPPET_END(round_f8)

// --------------------------------------------------------------------------
// Multiply by a floating-point constant. The constant lives in a literal
// pool appended to the kernel by the codegen, and the LDR's imm19 is
// patched at fixup time to point at the right pool entry.
//
// The LDR sentinel encodes imm19 = 0x7FFFF (the largest legal forward
// offset) so the placeholder is distinguishable from any LDR the codegen
// might emit elsewhere.
// --------------------------------------------------------------------------

SNIPPET(scale_f4)
    .inst   0x1CFFFFE1                  // ldr s1, [pc + 0x7FFFF*4]  (sentinel)
    fmul    s0, s0, s1
SNIPPET_END(scale_f4)

SNIPPET(scale_f8)
    .inst   0x5CFFFFE1                  // ldr d1, [pc + 0x7FFFF*4]  (sentinel)
    fmul    d0, d0, d1
SNIPPET_END(scale_f8)

// Clamp to a floating-point constant (float -> int saturation). The bound lives
// in the literal pool exactly like scale_f4/f8; the same LDR-imm19 sentinel is
// patched at fixup time. fmin/fmax give numeric min/max for finite inputs.

SNIPPET(min_f4)
    .inst   0x1CFFFFE1                  // ldr s1, [pc + sentinel]
    fmin    s0, s0, s1
SNIPPET_END(min_f4)

SNIPPET(min_f8)
    .inst   0x5CFFFFE1                  // ldr d1, [pc + sentinel]
    fmin    d0, d0, d1
SNIPPET_END(min_f8)

SNIPPET(max_f4)
    .inst   0x1CFFFFE1                  // ldr s1, [pc + sentinel]
    fmax    s0, s0, s1
SNIPPET_END(max_f4)

SNIPPET(max_f8)
    .inst   0x5CFFFFE1                  // ldr d1, [pc + sentinel]
    fmax    d0, d0, d1
SNIPPET_END(max_f8)

// --------------------------------------------------------------------------
// Default value materialization: load a field's raw default bits (zero-extended
// to 8 bytes) from the literal pool into x6. The matching integer store commits
// only the low bytes of the destination type. Uses the ldr-x literal sentinel
// (imm19 = 0x7FFFF), patched by the codegen.
// --------------------------------------------------------------------------

SNIPPET(load_imm)
    .inst   0x58FFFFE6                  // ldr x6, [pc + sentinel]
SNIPPET_END(load_imm)

// --------------------------------------------------------------------------
// 'Check' (assert) handling. The raw field bytes are loaded zero-extended into
// x6 (load_u1/u2/u4/u8 already zero-extend), byte-swapped if big-endian, and the
// expected bits loaded into x7 from the literal pool (load_imm_chk). chk_ne
// compares and branches forward to fail_postfix on mismatch (imm19 patched by
// the codegen).
// --------------------------------------------------------------------------

SNIPPET(load_imm_chk)
    .inst   0x58FFFFE7                  // ldr x7, [pc + sentinel]
SNIPPET_END(load_imm_chk)

SNIPPET(chk_ne)
    cmp     x6, x7
    .inst   0x54FFFFE1                  // b.ne fail (imm19 patch, sentinel)
SNIPPET_END(chk_ne)

// --------------------------------------------------------------------------
// Weight division (weighted -> unweighted). weight_recip computes the
// reciprocal of the loaded weight into v2 (reserved across the loop body):
// 1.0 if the weight is zero, else 1/w, with NaN propagated through the divide.
// weight_scale then multiplies the working value by that reciprocal. The
// branch is intra-snippet and PC-relative, so it survives the verbatim copy.
// --------------------------------------------------------------------------

SNIPPET(weight_recip_f4)
    fmov    s2, #1.0
    fcmp    s0, #0.0
    b.eq    1f
    fdiv    s2, s2, s0
1:
SNIPPET_END(weight_recip_f4)

SNIPPET(weight_recip_f8)
    fmov    d2, #1.0
    fcmp    d0, #0.0
    b.eq    1f
    fdiv    d2, d2, d0
1:
SNIPPET_END(weight_recip_f8)

SNIPPET(weight_scale_f4)
    fmul    s0, s0, s2
SNIPPET_END(weight_scale_f4)

SNIPPET(weight_scale_f8)
    fmul    d0, d0, d2
SNIPPET_END(weight_scale_f8)

// --------------------------------------------------------------------------
// Alpha (un)premultiplication. alpha_recip takes the linearized source alpha in
// v0 and stashes alpha in v16 and its reciprocal in v17 (0 if alpha is zero; a
// NaN alpha yields a NaN reciprocal). v16/v17 are reserved across the loop body,
// like the weight reciprocal in v2, and are caller-saved (v16-v31) so the leaf
// kernel need not preserve them. alpha_premul / alpha_unpremul multiply the
// working value by alpha / inv_alpha. The branch is intra-snippet, so it
// survives the verbatim copy.
// --------------------------------------------------------------------------

SNIPPET(alpha_recip_f4)
    fmov    s16, s0
    fmov    s17, #1.0
    fcmp    s0, #0.0
    b.eq    1f
    fdiv    s17, s17, s0
    b       2f
1:  fmov    s17, wzr
2:
SNIPPET_END(alpha_recip_f4)

SNIPPET(alpha_recip_f8)
    fmov    d16, d0
    fmov    d17, #1.0
    fcmp    d0, #0.0
    b.eq    1f
    fdiv    d17, d17, d0
    b       2f
1:  fmov    d17, xzr
2:
SNIPPET_END(alpha_recip_f8)

SNIPPET(alpha_premul_f4)
    fmul    s0, s0, s16
SNIPPET_END(alpha_premul_f4)

SNIPPET(alpha_premul_f8)
    fmul    d0, d0, d16
SNIPPET_END(alpha_premul_f8)

SNIPPET(alpha_unpremul_f4)
    fmul    s0, s0, s17
SNIPPET_END(alpha_unpremul_f4)

SNIPPET(alpha_unpremul_f8)
    fmul    d0, d0, d17
SNIPPET_END(alpha_unpremul_f8)

// --------------------------------------------------------------------------
// Blend accumulation. A blended output sums several weighted, linearized source
// terms in the working precision. blend_init seeds the accumulator (v7) with the
// first weighted term (already multiplied by its coefficient in v0); blend_add
// folds in each subsequent term; blend_fini moves the accumulator back into the
// working register v0 for the normal store path. v7 is scratch elsewhere but is
// not touched by the load / convert / scale / gamma snippets between terms.
// --------------------------------------------------------------------------

SNIPPET(blend_init_f4)
    fmov    s7, s0
SNIPPET_END(blend_init_f4)

SNIPPET(blend_init_f8)
    fmov    d7, d0
SNIPPET_END(blend_init_f8)

SNIPPET(blend_add_f4)
    fadd    s7, s7, s0
SNIPPET_END(blend_add_f4)

SNIPPET(blend_add_f8)
    fadd    d7, d7, d0
SNIPPET_END(blend_add_f8)

SNIPPET(blend_fini_f4)
    fmov    s0, s7
SNIPPET_END(blend_fini_f4)

SNIPPET(blend_fini_f8)
    fmov    d0, d7
SNIPPET_END(blend_fini_f8)

// --------------------------------------------------------------------------
// Gamma (sRGB) correction, rational polynomial (no calls), following Mitsuba's
// StructConverter::gamma(). The working value enters in v0 and the result leaves
// in v0. Coefficients live in the shared tail constant pool: `adr x7, .` is a
// placeholder whose 21-bit immediate the codegen patches to the table (one fixup
// per snippet), and the per-coefficient `[x7, #n]` offsets follow the flat
// gamma_coeffs.h layout [scale, threshold, num[0..k], den[0..k]].
//
// result = ratio(y) * x, where ratio(y) = num(y)/den(y) evaluated by Horner,
// y = x for sRGB->linear and y = sqrt(x) for linear->sRGB. For x below the
// threshold the linear segment scale*x is used instead. Registers: v1 = y,
// v3 = a (num, then ratio, then the multiplier), v4 = den, v5/v6 = coeff
// scratch, x7 = table base. v2 (weight reciprocal) is left untouched so gamma
// and weight division compose.
//
// One Horner step (num and den in lock-step), constants at [x7+\n] and [x7+\d].
.macro GMACC_S n, d
    ldr     s5, [x7, #\n]
    ldr     s6, [x7, #\d]
    fmadd   s3, s3, s1, s5
    fmadd   s4, s4, s1, s6
.endm
.macro GMACC_D n, d
    ldr     d5, [x7, #\n]
    ldr     d6, [x7, #\d]
    fmadd   d3, d3, d1, d5
    fmadd   d4, d4, d1, d6
.endm

SNIPPET(gamma_decode_f4)                 // sRGB -> linear (n = 5, y = x)
    adr     x7, .                        // table base (adr imm patched to pool)
    ldr     s3, [x7, #0]                 // a = 1/12.92
    ldr     s5, [x7, #4]                 // threshold
    fcmp    s0, s5
    b.lt    0f
    fmov    s1, s0                       // y = x
    ldr     s3, [x7, #8]                 // num[0]
    ldr     s4, [x7, #28]                // den[0]
    GMACC_S 12, 32
    GMACC_S 16, 36
    GMACC_S 20, 40
    GMACC_S 24, 44
    fdiv    s3, s3, s4
0:  fmul    s0, s0, s3
SNIPPET_END(gamma_decode_f4)

SNIPPET(gamma_encode_f4)                 // linear -> sRGB (n = 6, y = sqrt(x))
    adr     x7, .                        // table base (adr imm patched to pool)
    ldr     s3, [x7, #0]                 // a = 12.92
    ldr     s5, [x7, #4]                 // threshold
    fcmp    s0, s5
    b.lt    0f
    fsqrt   s1, s0                       // y = sqrt(x)
    ldr     s3, [x7, #8]                 // num[0]
    ldr     s4, [x7, #32]                // den[0]
    GMACC_S 12, 36
    GMACC_S 16, 40
    GMACC_S 20, 44
    GMACC_S 24, 48
    GMACC_S 28, 52
    fdiv    s3, s3, s4
0:  fmul    s0, s0, s3
SNIPPET_END(gamma_encode_f4)

SNIPPET(gamma_decode_f8)                 // sRGB -> linear (n = 10, y = x)
    adr     x7, .                        // table base (adr imm patched to pool)
    ldr     d3, [x7, #0]                 // a = 1/12.92
    ldr     d5, [x7, #8]                 // threshold
    fcmp    d0, d5
    b.lt    0f
    fmov    d1, d0                       // y = x
    ldr     d3, [x7, #16]                // num[0]
    ldr     d4, [x7, #96]                // den[0]
    GMACC_D 24, 104
    GMACC_D 32, 112
    GMACC_D 40, 120
    GMACC_D 48, 128
    GMACC_D 56, 136
    GMACC_D 64, 144
    GMACC_D 72, 152
    GMACC_D 80, 160
    GMACC_D 88, 168
    fdiv    d3, d3, d4
0:  fmul    d0, d0, d3
SNIPPET_END(gamma_decode_f8)

SNIPPET(gamma_encode_f8)                 // linear -> sRGB (n = 11, y = sqrt(x))
    adr     x7, .                        // table base (adr imm patched to pool)
    ldr     d3, [x7, #0]                 // a = 12.92
    ldr     d5, [x7, #8]                 // threshold
    fcmp    d0, d5
    b.lt    0f
    fsqrt   d1, d0                       // y = sqrt(x)
    ldr     d3, [x7, #16]                // num[0]
    ldr     d4, [x7, #104]               // den[0]
    GMACC_D 24, 112
    GMACC_D 32, 120
    GMACC_D 40, 128
    GMACC_D 48, 136
    GMACC_D 56, 144
    GMACC_D 64, 152
    GMACC_D 72, 160
    GMACC_D 80, 168
    GMACC_D 88, 176
    GMACC_D 96, 184
    fdiv    d3, d3, d4
0:  fmul    d0, d0, d3
SNIPPET_END(gamma_encode_f8)

// --------------------------------------------------------------------------
// Ordered dithering: add dither_matrix256[(y%256)*256 + (x%256)] to the working
// value before quantization. The table base is loaded from the literal pool
// (ldr-literal sentinel, patched by the codegen). x = x5, y = x4; w9/w10/x10 are
// scratch GPRs unused by the loop.
// --------------------------------------------------------------------------

SNIPPET(dither_f4)
    and     w9, w5, #0xff               // x % 256
    and     w10, w4, #0xff              // y % 256
    orr     w9, w9, w10, lsl #8         // idx = (y%256)*256 + (x%256)
    .inst   0x58FFFFEA                  // ldr x10, [pc + sentinel]  (table base)
    ldr     s5, [x10, w9, uxtw #2]
    fadd    s0, s0, s5
SNIPPET_END(dither_f4)

SNIPPET(dither_f8)
    and     w9, w5, #0xff
    and     w10, w4, #0xff
    orr     w9, w9, w10, lsl #8
    .inst   0x58FFFFEA                  // ldr x10, [pc + sentinel]
    ldr     s5, [x10, w9, uxtw #2]
    fcvt    d5, s5                      // table value is f32; widen to f64
    fadd    d0, d0, d5
SNIPPET_END(dither_f8)
