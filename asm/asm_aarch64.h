#pragma once

#include <stdint.h>
#include <stddef.h>

// Snippets are emitted as global symbols by asm/asm_aarch64.S. The size of
// each snippet is computed at runtime as (name_end - name).

extern "C" {
    #define SJIT_SNIPPET_DECL(name) \
        extern const uint8_t name[]; \
        extern const uint8_t name##_end[];

    SJIT_SNIPPET_DECL(main_prefix)
    SJIT_SNIPPET_DECL(main_start)
    SJIT_SNIPPET_DECL(main_inc_in)
    SJIT_SNIPPET_DECL(main_inc_out)
    SJIT_SNIPPET_DECL(main_suffix_inner)
    SJIT_SNIPPET_DECL(main_suffix_outer)
    SJIT_SNIPPET_DECL(main_postfix)
    SJIT_SNIPPET_DECL(fail_postfix)

    SJIT_SNIPPET_DECL(load_u1)
    SJIT_SNIPPET_DECL(load_u2)
    SJIT_SNIPPET_DECL(load_u4)
    SJIT_SNIPPET_DECL(load_u8)
    SJIT_SNIPPET_DECL(load_u2_unaligned)
    SJIT_SNIPPET_DECL(load_u4_unaligned)
    SJIT_SNIPPET_DECL(load_u8_unaligned)
    SJIT_SNIPPET_DECL(load_f2)
    SJIT_SNIPPET_DECL(load_f4)
    SJIT_SNIPPET_DECL(load_f8)
    SJIT_SNIPPET_DECL(load_f2_unaligned)
    SJIT_SNIPPET_DECL(load_f4_unaligned)
    SJIT_SNIPPET_DECL(load_f8_unaligned)

    SJIT_SNIPPET_DECL(store_u1)
    SJIT_SNIPPET_DECL(store_u2)
    SJIT_SNIPPET_DECL(store_u4)
    SJIT_SNIPPET_DECL(store_u8)
    SJIT_SNIPPET_DECL(store_u2_unaligned)
    SJIT_SNIPPET_DECL(store_u4_unaligned)
    SJIT_SNIPPET_DECL(store_u8_unaligned)
    SJIT_SNIPPET_DECL(store_f2)
    SJIT_SNIPPET_DECL(store_f4)
    SJIT_SNIPPET_DECL(store_f8)
    SJIT_SNIPPET_DECL(store_f2_unaligned)
    SJIT_SNIPPET_DECL(store_f4_unaligned)
    SJIT_SNIPPET_DECL(store_f8_unaligned)

    SJIT_SNIPPET_DECL(bswap_u2)
    SJIT_SNIPPET_DECL(bswap_u4)
    SJIT_SNIPPET_DECL(bswap_u8)

    SJIT_SNIPPET_DECL(mov_u2_f2)
    SJIT_SNIPPET_DECL(mov_f2_u2)
    SJIT_SNIPPET_DECL(mov_u4_f4)
    SJIT_SNIPPET_DECL(mov_f4_u4)
    SJIT_SNIPPET_DECL(mov_u8_f8)
    SJIT_SNIPPET_DECL(mov_f8_u8)

    SJIT_SNIPPET_DECL(cvt_u1_f4)
    SJIT_SNIPPET_DECL(cvt_i1_f4)
    SJIT_SNIPPET_DECL(cvt_u2_f4)
    SJIT_SNIPPET_DECL(cvt_i2_f4)
    SJIT_SNIPPET_DECL(cvt_u4_f4)
    SJIT_SNIPPET_DECL(cvt_i4_f4)
    SJIT_SNIPPET_DECL(cvt_u8_f4)
    SJIT_SNIPPET_DECL(cvt_i8_f4)
    SJIT_SNIPPET_DECL(cvt_u1_f8)
    SJIT_SNIPPET_DECL(cvt_i1_f8)
    SJIT_SNIPPET_DECL(cvt_u2_f8)
    SJIT_SNIPPET_DECL(cvt_i2_f8)
    SJIT_SNIPPET_DECL(cvt_u4_f8)
    SJIT_SNIPPET_DECL(cvt_i4_f8)
    SJIT_SNIPPET_DECL(cvt_u8_f8)
    SJIT_SNIPPET_DECL(cvt_i8_f8)

    SJIT_SNIPPET_DECL(cvt_f4_f8)
    SJIT_SNIPPET_DECL(cvt_f8_f4)

    SJIT_SNIPPET_DECL(cvt_f4_iw)
    SJIT_SNIPPET_DECL(cvt_f4_uw)
    SJIT_SNIPPET_DECL(cvt_f4_ix)
    SJIT_SNIPPET_DECL(cvt_f4_ux)
    SJIT_SNIPPET_DECL(cvt_f8_iw)
    SJIT_SNIPPET_DECL(cvt_f8_uw)
    SJIT_SNIPPET_DECL(cvt_f8_ix)
    SJIT_SNIPPET_DECL(cvt_f8_ux)

    SJIT_SNIPPET_DECL(round_f4)
    SJIT_SNIPPET_DECL(round_f8)
    SJIT_SNIPPET_DECL(scale_f4)
    SJIT_SNIPPET_DECL(scale_f8)
    SJIT_SNIPPET_DECL(min_f4)
    SJIT_SNIPPET_DECL(min_f8)
    SJIT_SNIPPET_DECL(max_f4)
    SJIT_SNIPPET_DECL(max_f8)
    SJIT_SNIPPET_DECL(load_imm)
    SJIT_SNIPPET_DECL(load_imm_chk)
    SJIT_SNIPPET_DECL(chk_ne)
    SJIT_SNIPPET_DECL(weight_recip_f4)
    SJIT_SNIPPET_DECL(weight_recip_f8)
    SJIT_SNIPPET_DECL(weight_scale_f4)
    SJIT_SNIPPET_DECL(weight_scale_f8)

    SJIT_SNIPPET_DECL(alpha_recip_f4)
    SJIT_SNIPPET_DECL(alpha_recip_f8)
    SJIT_SNIPPET_DECL(alpha_premul_f4)
    SJIT_SNIPPET_DECL(alpha_premul_f8)
    SJIT_SNIPPET_DECL(alpha_unpremul_f4)
    SJIT_SNIPPET_DECL(alpha_unpremul_f8)

    SJIT_SNIPPET_DECL(blend_init_f4)
    SJIT_SNIPPET_DECL(blend_init_f8)
    SJIT_SNIPPET_DECL(blend_add_f4)
    SJIT_SNIPPET_DECL(blend_add_f8)
    SJIT_SNIPPET_DECL(blend_fini_f4)
    SJIT_SNIPPET_DECL(blend_fini_f8)

    SJIT_SNIPPET_DECL(gamma_decode_f4)
    SJIT_SNIPPET_DECL(gamma_encode_f4)
    SJIT_SNIPPET_DECL(gamma_decode_f8)
    SJIT_SNIPPET_DECL(gamma_encode_f8)
    SJIT_SNIPPET_DECL(dither_f4)
    SJIT_SNIPPET_DECL(dither_f8)

    #undef SJIT_SNIPPET_DECL
}

// The impl enum mirrors the labels in asm_aarch64.S. Names match the
// auto-generated x86 enum so create_kernel can be shared between backends.

enum class impl : uint32_t {
    main_prefix,
    main_start,
    main_inc_in,
    main_inc_out,
    main_suffix_inner,
    main_suffix_outer,
    main_postfix,
    fail_postfix,

    load_u1, load_u2, load_u4, load_u8,
    load_u2_unaligned, load_u4_unaligned, load_u8_unaligned,
    load_f2, load_f4, load_f8,
    load_f2_unaligned, load_f4_unaligned, load_f8_unaligned,

    store_u1, store_u2, store_u4, store_u8,
    store_u2_unaligned, store_u4_unaligned, store_u8_unaligned,
    store_f2, store_f4, store_f8,
    store_f2_unaligned, store_f4_unaligned, store_f8_unaligned,

    bswap_u2, bswap_u4, bswap_u8,

    mov_u2_f2, mov_f2_u2,
    mov_u4_f4, mov_f4_u4,
    mov_u8_f8, mov_f8_u8,

    cvt_u1_f4, cvt_i1_f4,
    cvt_u2_f4, cvt_i2_f4,
    cvt_u4_f4, cvt_i4_f4,
    cvt_u8_f4, cvt_i8_f4,
    cvt_u1_f8, cvt_i1_f8,
    cvt_u2_f8, cvt_i2_f8,
    cvt_u4_f8, cvt_i4_f8,
    cvt_u8_f8, cvt_i8_f8,

    cvt_f4_f8, cvt_f8_f4,

    cvt_f4_iw, cvt_f4_uw, cvt_f4_ix, cvt_f4_ux,
    cvt_f8_iw, cvt_f8_uw, cvt_f8_ix, cvt_f8_ux,

    round_f4, round_f8,
    scale_f4, scale_f8,
    min_f4, min_f8,
    max_f4, max_f8,
    load_imm,
    load_imm_chk, chk_ne,
    weight_recip_f4, weight_recip_f8,
    weight_scale_f4, weight_scale_f8,

    alpha_recip_f4, alpha_recip_f8,
    alpha_premul_f4, alpha_premul_f8,
    alpha_unpremul_f4, alpha_unpremul_f8,

    blend_init_f4, blend_init_f8,
    blend_add_f4, blend_add_f8,
    blend_fini_f4, blend_fini_f8,

    gamma_decode_f4, gamma_encode_f4,
    gamma_decode_f8, gamma_encode_f8,
    dither_f4, dither_f8,

    last
};

struct SnippetSpan {
    const uint8_t *data;
    size_t size;
};

inline SnippetSpan get_snippet(impl i) {
    #define CASE(name) \
        case impl::name: \
            return SnippetSpan{ name, (size_t)(name##_end - name) };

    switch (i) {
        CASE(main_prefix)
        CASE(main_start)
        CASE(main_inc_in)
        CASE(main_inc_out)
        CASE(main_suffix_inner)
        CASE(main_suffix_outer)
        CASE(main_postfix)
        CASE(fail_postfix)

        CASE(load_u1)  CASE(load_u2)  CASE(load_u4)  CASE(load_u8)
        CASE(load_u2_unaligned) CASE(load_u4_unaligned) CASE(load_u8_unaligned)
        CASE(load_f2)  CASE(load_f4)  CASE(load_f8)
        CASE(load_f2_unaligned) CASE(load_f4_unaligned) CASE(load_f8_unaligned)

        CASE(store_u1) CASE(store_u2) CASE(store_u4) CASE(store_u8)
        CASE(store_u2_unaligned) CASE(store_u4_unaligned) CASE(store_u8_unaligned)
        CASE(store_f2) CASE(store_f4) CASE(store_f8)
        CASE(store_f2_unaligned) CASE(store_f4_unaligned) CASE(store_f8_unaligned)

        CASE(bswap_u2) CASE(bswap_u4) CASE(bswap_u8)

        CASE(mov_u2_f2) CASE(mov_f2_u2)
        CASE(mov_u4_f4) CASE(mov_f4_u4)
        CASE(mov_u8_f8) CASE(mov_f8_u8)

        CASE(cvt_u1_f4) CASE(cvt_i1_f4)
        CASE(cvt_u2_f4) CASE(cvt_i2_f4)
        CASE(cvt_u4_f4) CASE(cvt_i4_f4)
        CASE(cvt_u8_f4) CASE(cvt_i8_f4)
        CASE(cvt_u1_f8) CASE(cvt_i1_f8)
        CASE(cvt_u2_f8) CASE(cvt_i2_f8)
        CASE(cvt_u4_f8) CASE(cvt_i4_f8)
        CASE(cvt_u8_f8) CASE(cvt_i8_f8)

        CASE(cvt_f4_f8) CASE(cvt_f8_f4)

        CASE(cvt_f4_iw) CASE(cvt_f4_uw) CASE(cvt_f4_ix) CASE(cvt_f4_ux)
        CASE(cvt_f8_iw) CASE(cvt_f8_uw) CASE(cvt_f8_ix) CASE(cvt_f8_ux)

        CASE(round_f4) CASE(round_f8)
        CASE(scale_f4) CASE(scale_f8)
        CASE(min_f4) CASE(min_f8)
        CASE(max_f4) CASE(max_f8)
        CASE(load_imm)
        CASE(load_imm_chk) CASE(chk_ne)
        CASE(weight_recip_f4) CASE(weight_recip_f8)
        CASE(weight_scale_f4) CASE(weight_scale_f8)

        CASE(alpha_recip_f4) CASE(alpha_recip_f8)
        CASE(alpha_premul_f4) CASE(alpha_premul_f8)
        CASE(alpha_unpremul_f4) CASE(alpha_unpremul_f8)

        CASE(blend_init_f4) CASE(blend_init_f8)
        CASE(blend_add_f4) CASE(blend_add_f8)
        CASE(blend_fini_f4) CASE(blend_fini_f8)

        CASE(gamma_decode_f4) CASE(gamma_encode_f4)
        CASE(gamma_decode_f8) CASE(gamma_encode_f8)
        CASE(dither_f4) CASE(dither_f8)

        default:
            return SnippetSpan{ nullptr, 0 };
    }
    #undef CASE
}

// Patch kinds. Indicates how to splice an operand into a snippet's
// placeholder bits.

enum class PatchKind : uint32_t {
    None,
    Imm12_S1,    // 12-bit unsigned immediate, scale 1 (ldrb / strb)
    Imm12_S2,    // 12-bit unsigned immediate, scale 2 (ldrh / strh / ldr h)
    Imm12_S4,    // 12-bit unsigned immediate, scale 4 (ldr/str w / ldr s)
    Imm12_S8,    // 12-bit unsigned immediate, scale 8 (ldr/str x / ldr d)
    Imm9_Unscaled, // 9-bit signed immediate, scale 1 (ldur/stur family)
    Imm12_Add,   // 12-bit unsigned immediate, unscaled (add immediate)
    Imm19_BCond  // 19-bit signed PC-relative offset (b.cond)
};

inline PatchKind patch_kind(impl i) {
    switch (i) {
        case impl::load_u1:
        case impl::store_u1:
            return PatchKind::Imm12_S1;

        case impl::load_u2:
        case impl::store_u2:
        case impl::load_f2:
        case impl::store_f2:
            return PatchKind::Imm12_S2;

        case impl::load_u4:
        case impl::store_u4:
        case impl::load_f4:
        case impl::store_f4:
            return PatchKind::Imm12_S4;

        case impl::load_u8:
        case impl::store_u8:
        case impl::load_f8:
        case impl::store_f8:
            return PatchKind::Imm12_S8;

        case impl::load_u2_unaligned:
        case impl::load_u4_unaligned:
        case impl::load_u8_unaligned:
        case impl::load_f2_unaligned:
        case impl::load_f4_unaligned:
        case impl::load_f8_unaligned:
        case impl::store_u2_unaligned:
        case impl::store_u4_unaligned:
        case impl::store_u8_unaligned:
        case impl::store_f2_unaligned:
        case impl::store_f4_unaligned:
        case impl::store_f8_unaligned:
            return PatchKind::Imm9_Unscaled;

        case impl::main_inc_in:
        case impl::main_inc_out:
            return PatchKind::Imm12_Add;

        case impl::main_suffix_inner:
        case impl::main_suffix_outer:
            return PatchKind::Imm19_BCond;

        // scale_f4 / scale_f8 don't go through put()'s patcher; the codegen
        // calls emit_scale() directly, which appends the constant to the
        // tail literal pool and queues a separate LDR-imm19 fixup.

        default:
            return PatchKind::None;
    }
}
