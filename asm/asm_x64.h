#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(__APPLE__)
#  define SJIT_ASM_SYMBOL(name) __asm__(#name)
#else
#  define SJIT_ASM_SYMBOL(name)
#endif

extern "C" {
    #define SJIT_SNIPPET_DECL(name) \
        extern const uint8_t name[] SJIT_ASM_SYMBOL(name); \
        extern const uint8_t name##_end[] SJIT_ASM_SYMBOL(name##_end);

    #define SJIT_LABEL_DECL(name) \
        extern const uint8_t name[] SJIT_ASM_SYMBOL(name);

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
    SJIT_SNIPPET_DECL(load_f2)
    SJIT_SNIPPET_DECL(load_f4)
    SJIT_SNIPPET_DECL(load_f8)

    SJIT_SNIPPET_DECL(store_u1)
    SJIT_SNIPPET_DECL(store_u2)
    SJIT_SNIPPET_DECL(store_u4)
    SJIT_SNIPPET_DECL(store_u8)
    SJIT_SNIPPET_DECL(store_f2)
    SJIT_SNIPPET_DECL(store_f4)
    SJIT_SNIPPET_DECL(store_f8)

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
    SJIT_SNIPPET_DECL(cvt_u1_f8)
    SJIT_SNIPPET_DECL(cvt_i1_f8)
    SJIT_SNIPPET_DECL(cvt_u2_f4)
    SJIT_SNIPPET_DECL(cvt_i2_f4)
    SJIT_SNIPPET_DECL(cvt_u2_f8)
    SJIT_SNIPPET_DECL(cvt_i2_f8)
    SJIT_SNIPPET_DECL(cvt_u4_f4)
    SJIT_SNIPPET_DECL(cvt_i4_f4)
    SJIT_SNIPPET_DECL(cvt_u4_f8)
    SJIT_SNIPPET_DECL(cvt_i4_f8)
    SJIT_SNIPPET_DECL(cvt_u8_f8)
    SJIT_SNIPPET_DECL(cvt_i8_f8)
    SJIT_SNIPPET_DECL(cvt_u8_f4)
    SJIT_SNIPPET_DECL(cvt_i8_f4)

    SJIT_LABEL_DECL(cvt_f4_u1)
    SJIT_LABEL_DECL(cvt_f4_i1)
    SJIT_LABEL_DECL(cvt_f4_u2)
    SJIT_LABEL_DECL(cvt_f4_i2)
    SJIT_LABEL_DECL(cvt_f4_i4)
    SJIT_LABEL_DECL(cvt_f4_u4)
    SJIT_LABEL_DECL(cvt_f4_i8)
    SJIT_SNIPPET_DECL(cvt_f4_u8)

    SJIT_LABEL_DECL(cvt_f8_u1)
    SJIT_LABEL_DECL(cvt_f8_i1)
    SJIT_LABEL_DECL(cvt_f8_u2)
    SJIT_LABEL_DECL(cvt_f8_i2)
    SJIT_LABEL_DECL(cvt_f8_i4)
    SJIT_LABEL_DECL(cvt_f8_u4)
    SJIT_LABEL_DECL(cvt_f8_i8)
    SJIT_SNIPPET_DECL(cvt_f8_u8)

    SJIT_SNIPPET_DECL(cvt_f4_f8)
    SJIT_SNIPPET_DECL(cvt_f8_f4)
    SJIT_SNIPPET_DECL(scale_f4)
    SJIT_SNIPPET_DECL(scale_f8)
    SJIT_SNIPPET_DECL(min_f4)
    SJIT_SNIPPET_DECL(min_f8)
    SJIT_SNIPPET_DECL(max_f4)
    SJIT_SNIPPET_DECL(max_f8)
    SJIT_SNIPPET_DECL(round_f4)
    SJIT_SNIPPET_DECL(round_f8)
    SJIT_SNIPPET_DECL(load_imm)
    SJIT_SNIPPET_DECL(chk_zload_u1)
    SJIT_SNIPPET_DECL(chk_zload_u2)
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

    extern const uint8_t cvt_f4_u1_end[] SJIT_ASM_SYMBOL(cvt_f4_u1_end);
    extern const uint8_t cvt_f4_u4_end[] SJIT_ASM_SYMBOL(cvt_f4_u4_end);
    extern const uint8_t cvt_f8_u1_end[] SJIT_ASM_SYMBOL(cvt_f8_u1_end);
    extern const uint8_t cvt_f8_u4_end[] SJIT_ASM_SYMBOL(cvt_f8_u4_end);

    #undef SJIT_LABEL_DECL
    #undef SJIT_SNIPPET_DECL
}

#undef SJIT_ASM_SYMBOL

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
    load_f2, load_f4, load_f8,

    store_u1, store_u2, store_u4, store_u8,
    store_f2, store_f4, store_f8,

    bswap_u2, bswap_u4, bswap_u8,

    mov_u2_f2, mov_f2_u2,
    mov_u4_f4, mov_f4_u4,
    mov_u8_f8, mov_f8_u8,

    cvt_u1_f4, cvt_i1_f4,
    cvt_u1_f8, cvt_i1_f8,
    cvt_u2_f4, cvt_i2_f4,
    cvt_u2_f8, cvt_i2_f8,
    cvt_u4_f4, cvt_i4_f4,
    cvt_u4_f8, cvt_i4_f8,
    cvt_u8_f8, cvt_i8_f8,
    cvt_u8_f4, cvt_i8_f4,

    cvt_f4_u1, cvt_f4_i1, cvt_f4_u2, cvt_f4_i2, cvt_f4_i4,
    cvt_f4_u4, cvt_f4_i8, cvt_f4_u8,
    cvt_f8_u1, cvt_f8_i1, cvt_f8_u2, cvt_f8_i2, cvt_f8_i4,
    cvt_f8_u4, cvt_f8_i8, cvt_f8_u8,

    cvt_f4_f8, cvt_f8_f4,
    scale_f4, scale_f8,
    min_f4, min_f8,
    max_f4, max_f8,
    round_f4, round_f8,
    load_imm,
    chk_zload_u1, chk_zload_u2, load_imm_chk, chk_ne,
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
            return SnippetSpan{ name, (size_t) (name##_end - name) };

    #define CASE_SHARED(name, end) \
        case impl::name: \
            return SnippetSpan{ name, (size_t) (end - name) };

    switch (i) {
        CASE(main_prefix)
        CASE(main_start)
        CASE(main_inc_in)
        CASE(main_inc_out)
        CASE(main_suffix_inner)
        CASE(main_suffix_outer)
        CASE(main_postfix)
        CASE(fail_postfix)

        CASE(load_u1) CASE(load_u2) CASE(load_u4) CASE(load_u8)
        CASE(load_f2) CASE(load_f4) CASE(load_f8)

        CASE(store_u1) CASE(store_u2) CASE(store_u4) CASE(store_u8)
        CASE(store_f2) CASE(store_f4) CASE(store_f8)

        CASE(bswap_u2) CASE(bswap_u4) CASE(bswap_u8)

        CASE(mov_u2_f2) CASE(mov_f2_u2)
        CASE(mov_u4_f4) CASE(mov_f4_u4)
        CASE(mov_u8_f8) CASE(mov_f8_u8)

        CASE(cvt_u1_f4) CASE(cvt_i1_f4)
        CASE(cvt_u1_f8) CASE(cvt_i1_f8)
        CASE(cvt_u2_f4) CASE(cvt_i2_f4)
        CASE(cvt_u2_f8) CASE(cvt_i2_f8)
        CASE(cvt_u4_f4) CASE(cvt_i4_f4)
        CASE(cvt_u4_f8) CASE(cvt_i4_f8)
        CASE(cvt_u8_f8) CASE(cvt_i8_f8)
        CASE(cvt_u8_f4) CASE(cvt_i8_f4)

        CASE_SHARED(cvt_f4_u1, cvt_f4_u1_end)
        CASE_SHARED(cvt_f4_i1, cvt_f4_u1_end)
        CASE_SHARED(cvt_f4_u2, cvt_f4_u1_end)
        CASE_SHARED(cvt_f4_i2, cvt_f4_u1_end)
        CASE_SHARED(cvt_f4_i4, cvt_f4_u1_end)
        CASE_SHARED(cvt_f4_u4, cvt_f4_u4_end)
        CASE_SHARED(cvt_f4_i8, cvt_f4_u4_end)
        CASE(cvt_f4_u8)

        CASE_SHARED(cvt_f8_u1, cvt_f8_u1_end)
        CASE_SHARED(cvt_f8_i1, cvt_f8_u1_end)
        CASE_SHARED(cvt_f8_u2, cvt_f8_u1_end)
        CASE_SHARED(cvt_f8_i2, cvt_f8_u1_end)
        CASE_SHARED(cvt_f8_i4, cvt_f8_u1_end)
        CASE_SHARED(cvt_f8_u4, cvt_f8_u4_end)
        CASE_SHARED(cvt_f8_i8, cvt_f8_u4_end)
        CASE(cvt_f8_u8)

        CASE(cvt_f4_f8) CASE(cvt_f8_f4)
        CASE(scale_f4) CASE(scale_f8)
        CASE(min_f4) CASE(min_f8)
        CASE(max_f4) CASE(max_f8)
        CASE(round_f4) CASE(round_f8)
        CASE(load_imm)
        CASE(chk_zload_u1) CASE(chk_zload_u2) CASE(load_imm_chk) CASE(chk_ne)
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

    #undef CASE_SHARED
    #undef CASE
}
