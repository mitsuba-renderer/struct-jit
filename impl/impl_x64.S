.intel_syntax
.text

# void convert(size_t height, size_t width, const void *src, void *dst) {
#     for (size_t y = 0; y < height; ++y) {
#         for (size_t x = 0; x < width; ++x) {
#             for (each field) {
#             }
#         }
#     }
# }

# [ ] Gamma-correct, gamma-uncorrect
# [x] Normalize, unnormlalize
# [x] Weight division
# [ ] Dithering
# [ ] Alpha premult, unpremult
# [x] Endianness

# Loop body setup
main_prefix:
    cc_prefix
    xor y, y
main_prefix_end:
    .size main_prefix, main_prefix_end-main_prefix

main_init_idx:
    xor idx, idx
main_init_idx_end:
    .size main_init_idx, main_init_idx_end-main_init_idx

main_start: # Outer loop start
    xor x, x
main_start_end:
    .size main_start, main_start_end-main_start

# << Loop body goes here >>

main_inc_src:
    add src, extra
main_inc_src_end:
    .size main_inc_src, main_inc_src_end-main_inc_src

main_inc_dst:
    add dst, extra
main_inc_dst_end:
    .size main_inc_dst, main_inc_dst_end-main_inc_dst

main_inc_idx:
    inc idx
main_inc_idx_end:
    .size main_inc_idx, main_inc_idx_end-main_inc_idx

main_suffix_inner:
    inc x
    cmp x, width
    jne [main_suffix_inner_end + extra] # -> loop body
main_suffix_inner_end:
    .size main_suffix_inner, main_suffix_inner_end-main_suffix_inner

main_suffix_outer:
    inc y
    cmp y, height
    jne [main_suffix_outer_end + extra] # -> main_start
main_suffix_outer_end:
    .size main_suffix_outer, main_suffix_outer_end-main_suffix_outer

main_postfix:
    cc_postfix
    ret
main_postfix_end:
    .size main_postfix, main_postfix_end-main_postfix

# ====> Load direct

load_u1:
    mov al, byte ptr [src + extra]
load_u1_end:
    .size load_u1, load_u1_end-load_u1

load_u2:
    mov ax, word ptr [src + extra]
load_u2_end:
    .size load_u2, load_u2_end-load_u2

load_u4:
    mov eax, dword ptr [src + extra]
load_u4_end:
    .size load_u4, load_u4_end-load_u4

load_u8:
    mov rax, qword ptr [src + extra]
load_u8_end:
    .size load_u8, load_u8_end-load_u8

load_f2:
    movzx eax, word ptr [src + extra]
    vmovd xmm0, eax
    vcvtph2ps xmm0, xmm0
load_f2_end:
    .size load_f2, load_f2_end-load_f2

load_f4:
    vmovss xmm0, dword ptr [src + extra]
load_f4_end:
    .size load_f4, load_f4_end-load_f4

load_f8:
    vmovsd xmm0, qword ptr [src + extra]
load_f8_end:
    .size load_f8, load_f8_end-load_f8

; cvt_f4_f2:
; cvt_f4_f2_end:
;     .size cvt_f4_f2, cvt_f4_f2_end-cvt_f4_f2
;
;

# ====> Load indirect

loadi_u1:
    mov tmp, qword ptr [src + extra]
    mov al, byte ptr [tmp + idx]
loadi_u1_end:
    .size loadi_u1, loadi_u1_end-loadi_u1

loadi_u2:
    mov tmp, qword ptr [src + extra]
    mov ax, word ptr [tmp + 2*idx]
loadi_u2_end:
    .size loadi_u2, loadi_u2_end-loadi_u2

loadi_u4:
    mov tmp, qword ptr [src + extra]
    mov eax, dword ptr [tmp + 4*idx]
loadi_u4_end:
    .size loadi_u4, loadi_u4_end-loadi_u4

loadi_u8:
    mov tmp, qword ptr [src + extra]
    mov rax, qword ptr [tmp + 8*idx]
loadi_u8_end:
    .size loadi_u8, loadi_u8_end-loadi_u8

loadi_f2:
    mov tmp, qword ptr [src + extra]
    movzx eax, word ptr [tmp + 2*idx]
    vmovd xmm0, eax
    vcvtph2ps xmm0, xmm0
loadi_f2_end:
    .size loadi_f2, loadi_f2_end-loadi_f2

loadi_f4:
    mov tmp, qword ptr [src + extra]
    vmovss xmm0, dword ptr [tmp + 4*idx]
loadi_f4_end:
    .size loadi_f4, loadi_f4_end-loadi_f4

loadi_f8:
    mov tmp, qword ptr [src + extra]
    vmovsd xmm0, qword ptr [tmp + 8*idx]
loadi_f8_end:
    .size loadi_f8, loadi_f8_end-loadi_f8

# ====> Store direct

store_u1:
    mov byte ptr [dst + extra], al
store_u1_end:
    .size store_u1, store_u1_end-store_u1

store_u2:
    mov word ptr [dst + extra], ax
store_u2_end:
    .size store_u2, store_u2_end-store_u2

store_u4:
    mov dword ptr [dst + extra], eax
store_u4_end:
    .size store_u4, store_u4_end-store_u4

store_u8:
    mov qword ptr [dst + extra], rax
store_u8_end:
    .size store_u8, store_u8_end-store_u8

store_f2:
    vcvtps2ph xmm0, xmm0, 0
    vmovd eax, xmm0
    mov word ptr [dst + extra], ax
store_f2_end:
    .size store_f2, store_f2_end-store_f2

store_f4:
    vmovss dword ptr [dst + extra], xmm0
store_f4_end:
    .size store_f4, store_f4_end-store_f4

store_f8:
    vmovsd qword ptr [dst + extra], xmm0
store_f8_end:
    .size store_f8, store_f8_end-store_f8

# ====> Store indirect

storei_u1:
    mov tmp, qword ptr [dst + extra]
    mov byte ptr [tmp + idx], al
storei_u1_end:
    .size storei_u1, storei_u1_end-storei_u1

storei_u2:
    mov tmp, qword ptr [dst + extra]
    mov word ptr [tmp + 2*idx], ax
storei_u2_end:
    .size storei_u2, storei_u2_end-storei_u2

storei_u4:
    mov tmp, qword ptr [dst + extra]
    mov dword ptr [tmp + 4*idx], eax
storei_u4_end:
    .size storei_u4, storei_u4_end-storei_u4
storei_u8:
    mov tmp, qword ptr [dst + extra]
    mov qword ptr [tmp + 8*idx], rax
storei_u8_end:
    .size storei_u8, storei_u8_end-storei_u8

storei_f2:
    vcvtps2ph xmm0, xmm0, 0
    vmovd eax, xmm0
    mov tmp, qword ptr [dst + extra]
    mov word ptr [tmp + 2*idx], ax
storei_f2_end:
    .size storei_f2, storei_f2_end-storei_f2

storei_f4:
    mov tmp, qword ptr [dst + extra]
    vmovss dword ptr [tmp + 4*idx], xmm0
storei_f4_end:
    .size storei_f4, storei_f4_end-storei_f4

storei_f8:
    mov tmp, qword ptr [dst + extra]
    vmovsd qword ptr [tmp + 8*idx], xmm0
storei_f8_end:
    .size storei_f8, storei_f8_end-storei_f8

# ====> Endianness swap

bswap_1:
bswap_1_end:
    .size bswap_1, bswap_1_end-bswap_1

bswap_2:
    xchg ah, al
bswap_2_end:
    .size bswap_2, bswap_2_end-bswap_2

bswap_4:
    bswap eax
bswap_4_end:
    .size bswap_4, bswap_4_end-bswap_4

bswap_8:
    bswap rax
bswap_8_end:
    .size bswap_8, bswap_8_end-bswap_8

# ====> Integer -> Float conversion

cvt_u1_f4:
    movzx eax, al
    vcvtsi2ss xmm0, xmm0, eax
cvt_u1_f4_end:
    .size cvt_u1_f4, cvt_u1_f4_end-cvt_u1_f4

cvt_i1_f4:
    movsx eax, al
    vcvtsi2ss xmm0, xmm0, eax
cvt_i1_f4_end:
    .size cvt_i1_f4, cvt_i1_f4_end-cvt_i1_f4

cvt_u1_f8:
    movzx eax, al
    vcvtsi2sd xmm0, xmm0, eax
cvt_u1_f8_end:
    .size cvt_u1_f8, cvt_u1_f8_end-cvt_u1_f8

cvt_i1_f8:
    movsx eax, al
    vcvtsi2sd xmm0, xmm0, eax
cvt_i1_f8_end:
    .size cvt_i1_f8, cvt_i1_f8_end-cvt_i1_f8

cvt_u2_f4:
    movzx eax, ax
    vcvtsi2ss xmm0, xmm0, eax
cvt_u2_f4_end:
    .size cvt_u2_f4, cvt_u2_f4_end-cvt_u2_f4

cvt_i2_f4:
    movsx eax, ax
    vcvtsi2ss xmm0, xmm0, eax
cvt_i2_f4_end:
    .size cvt_i2_f4, cvt_i2_f4_end-cvt_i2_f4

cvt_u2_f8:
    movzx eax, ax
    vcvtsi2sd xmm0, xmm0, eax
cvt_u2_f8_end:
    .size cvt_u2_f8, cvt_u2_f8_end-cvt_u2_f8

cvt_i2_f8:
    movsx eax, ax
    vcvtsi2sd xmm0, xmm0, eax
cvt_i2_f8_end:
    .size cvt_i2_f8, cvt_i2_f8_end-cvt_i2_f8

cvt_u4_f4:
    vcvtsi2ss xmm0, xmm0, rax
cvt_u4_f4_end:
    .size cvt_u4_f4, cvt_u4_f4_end-cvt_u4_f4

cvt_i4_f4:
    vcvtsi2ss xmm0, xmm0, eax
cvt_i4_f4_end:
    .size cvt_i4_f4, cvt_i4_f4_end-cvt_i4_f4

cvt_u4_f8:
    vcvtsi2sd xmm0, xmm0, rax
cvt_u4_f8_end:
    .size cvt_u4_f8, cvt_u4_f8_end-cvt_u4_f8

cvt_i4_f8:
    vcvtsi2sd xmm0, xmm0, eax
cvt_i4_f8_end:
    .size cvt_i4_f8, cvt_i4_f8_end-cvt_i4_f8

cvt_u8_f4:
    test rax, rax
    js cvg_u8_f4_tmp
    vcvtsi2ss xmm0, xmm0, rax
    jmp cvt_u8_f4_end
cvg_u8_f4_tmp:
    shr rax
    vcvtsi2ss xmm0, xmm0, rax
    vaddss xmm0, xmm0, xmm0
cvt_u8_f4_end:
    .size cvt_u8_f4, cvt_u8_f4_end-cvt_u8_f4

cvt_i8_f4:
    vcvtsi2ss xmm0, xmm0, rax
cvt_i8_f4_end:
    .size cvt_i8_f4, cvt_i8_f4_end-cvt_i8_f4

cvt_u8_f8:
    test rax, rax
    js cvg_u8_f8_tmp
    vcvtsi2sd xmm0, xmm0, rax
    jmp cvt_u8_f8_end
cvg_u8_f8_tmp:
    shr rax
    vcvtsi2sd xmm0, xmm0, rax
    vaddsd xmm0, xmm0, xmm0
cvt_u8_f8_end:
    .size cvt_u8_f8, cvt_u8_f8_end-cvt_u8_f8

cvt_i8_f8:
    vcvtsi2sd xmm0, xmm0, rax
cvt_i8_f8_end:
    .size cvt_i8_f8, cvt_i8_f8_end-cvt_i8_f8

# ====> Float -> Integer conversion

cvt_f4_u1:
cvt_f4_i1:
cvt_f4_u2:
cvt_f4_i2:
cvt_f4_i4:
    vcvtss2si eax, xmm0
cvt_f4_u1_end:
    .size cvt_f4_u1, cvt_f4_u1_end-cvt_f4_u1
    .size cvt_f4_i1, cvt_f4_u1_end-cvt_f4_u1
    .size cvt_f4_u2, cvt_f4_u1_end-cvt_f4_u1
    .size cvt_f4_i2, cvt_f4_u1_end-cvt_f4_u1
    .size cvt_f4_i4, cvt_f4_u1_end-cvt_f4_u1

cvt_f4_u4:
cvt_f4_i8:
    vcvtss2si rax, xmm0
cvt_f4_u4_end:
    .size cvt_f4_u4, cvt_f4_u4_end-cvt_f4_u4
    .size cvt_f4_i8, cvt_f4_u4_end-cvt_f4_u4

cvt_f4_u8:
    mov tmp, 0x5f000000 # 9.22337e+18
    vmovd xmm1, tmp
    vucomiss xmm0, xmm1
    jnb cvt_f4_u8_tmp
    vcvtss2si rax, xmm0
    jmp cvt_f4_u8_end
cvt_f4_u8_tmp:
    vsubss xmm0, xmm0, xmm1
    vcvtss2si rax, xmm0
    btc rax, 63
cvt_f4_u8_end:
    .size cvt_f4_u8, cvt_f4_u8_end-cvt_f4_u8

cvt_f8_u1:
cvt_f8_i1:
cvt_f8_u2:
cvt_f8_i2:
cvt_f8_i4:
    vcvtsd2si eax, xmm0
cvt_f8_u1_end:
    .size cvt_f8_u1, cvt_f8_u1_end-cvt_f8_u1
    .size cvt_f8_i1, cvt_f8_u1_end-cvt_f8_u1
    .size cvt_f8_u2, cvt_f8_u1_end-cvt_f8_u1
    .size cvt_f8_i2, cvt_f8_u1_end-cvt_f8_u1
    .size cvt_f8_i4, cvt_f8_u1_end-cvt_f8_u1

cvt_f8_u4:
cvt_f8_i8:
    vcvtsd2si rax, xmm0
cvt_f8_u4_end:
    .size cvt_f8_u4, cvt_f8_u4_end-cvt_f8_u4
    .size cvt_f8_i8, cvt_f8_u4_end-cvt_f8_u4

cvt_f8_u8:
    mov tmp, 0x43e0000000000000 # 9.223372036854776e+18
    vmovq xmm1, tmp
    vucomisd xmm0, xmm1
    jnb cvt_f8_u8_tmp
    vcvtsd2si rax, xmm0
    jmp cvt_f8_u8_end
cvt_f8_u8_tmp:
    vsubsd xmm0, xmm0, xmm1
    vcvtsd2si rax, xmm0
    btc rax, 63
cvt_f8_u8_end:
    .size cvt_f8_u8, cvt_f8_u8_end-cvt_f8_u8

# ====> Float -> Float conversion

cvt_f4_f8:
    vcvtss2sd xmm0, xmm0, xmm0
cvt_f4_f8_end:
    .size cvt_f4_f8, cvt_f4_f8_end-cvt_f4_f8

cvt_f8_f4:
    vcvtsd2ss xmm0, xmm0, xmm0
cvt_f8_f4_end:
    .size cvt_f8_f4, cvt_f8_f4_end-cvt_f8_f4

# ====> Simple transformations

scale_f4:
    vmulss xmm0, xmm0, dword ptr [rip+extra]
scale_f4_end:
    .size scale_f4, scale_f4_end-scale_f4

scale_f8:
    vmulsd xmm0, xmm0, qword ptr [rip+extra]
scale_f8_end:
    .size scale_f8, scale_f8_end-scale_f8

min_f4:
    vminss xmm0, xmm0, dword ptr [rip+extra]
min_f4_end:
    .size min_f4, min_f4_end-min_f4

min_f8:
    vminsd xmm0, xmm0, qword ptr [rip+extra]
min_f8_end:
    .size min_f8, min_f8_end-min_f8

max_f4:
    vmaxss xmm0, xmm0, dword ptr [rip+extra]
max_f4_end:
    .size max_f4, max_f4_end-max_f4

max_f8:
    vmaxsd xmm0, xmm0, qword ptr [rip+extra]
max_f8_end:
    .size max_f8, max_f8_end-max_f8

round_f4:
    vroundss xmm0, xmm0, xmm0, 0
round_f4_end:
    .size round_f4, round_f4_end-round_f4

round_f8:
    vroundsd xmm0, xmm0, xmm0, 0
round_f8_end:
    .size round_f8, round_f8_end-round_f8

# ====> Weight handling

weight_recip_f4:
    mov tmp, 0x3f800000 # 1.0
    vmovd xmm1, tmp
    vdivss xmm2, xmm1, xmm0
weight_recip_f4_end:
    .size weight_recip_f4, weight_recip_f4_end-weight_recip_f4

weight_recip_f8:
    mov tmp, 0x3ff0000000000000 # 1.0
    vmovq xmm1, tmp
    vdivsd xmm2, xmm1, xmm0
weight_recip_f8_end:
    .size weight_recip_f8, weight_recip_f8_end-weight_recip_f8

weight_scale_f4:
    vmulss xmm0, xmm0, xmm2
weight_scale_f4_end:
    .size weight_scale_f4, weight_scale_f4_end-weight_scale_f4

weight_scale_f8:
    vmulsd xmm0, xmm0, xmm2
weight_scale_f8_end:
    .size weight_scale_f8, weight_scale_f8_end-weight_scale_f8
