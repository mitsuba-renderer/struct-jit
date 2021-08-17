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

# Gamma-correct, gamma-uncorrect
# Normalize, unnormlalize
# Weight division
# Dithering
# Alpha premult, unpremult
# Endianness


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

load_1:
    mov al, byte ptr [src + extra]
load_1_end:
    .size load_1, load_1_end-load_1

load_2:
    mov ax, word ptr [src + extra]
load_2_end:
    .size load_2, load_2_end-load_2

load_4:
    mov eax, dword ptr [src + extra]
load_4_end:
    .size load_4, load_4_end-load_4

load_8:
    mov rax, qword ptr [src + extra]
load_8_end:
    .size load_8, load_8_end-load_8

# ====> Load indirect

loadi_1:
    mov tmp, qword ptr [src + extra]
    mov al, byte ptr [tmp + idx]
loadi_1_end:
    .size loadi_1, loadi_1_end-loadi_1

loadi_2:
    mov tmp, qword ptr [src + extra]
    mov ax, word ptr [tmp + 2*idx]
loadi_2_end:
    .size loadi_2, loadi_2_end-loadi_2

loadi_4:
    mov tmp, qword ptr [src + extra]
    mov eax, dword ptr [tmp + 2*idx]
loadi_4_end:
    .size loadi_4, loadi_4_end-loadi_4

loadi_8:
    mov tmp, qword ptr [src + extra]
    mov rax, qword ptr [tmp + 4*idx]
loadi_8_end:
    .size loadi_8, loadi_8_end-loadi_8

# ====> Store direct

store_1:
    mov byte ptr [dst + extra], al
store_1_end:
    .size store_1, store_1_end-store_1

store_2:
    mov word ptr [dst + extra], ax
store_2_end:
    .size store_2, store_2_end-store_2

store_4:
    mov dword ptr [dst + extra], eax
store_4_end:
    .size store_4, store_4_end-store_4

store_8:
    mov qword ptr [dst + extra], rax
store_8_end:
    .size store_8, store_8_end-store_8

# ====> Store indirect

storei_1:
    mov tmp, qword ptr [dst + extra]
    mov byte ptr [tmp + idx], al
storei_1_end:
    .size storei_1, storei_1_end-storei_1

storei_2:
    mov tmp, qword ptr [dst + extra]
    mov word ptr [tmp + 2*idx], ax
storei_2_end:
    .size storei_2, storei_2_end-storei_2

storei_4:
    mov tmp, qword ptr [dst + extra]
    mov dword ptr [tmp + 2*idx], eax
storei_4_end:
    .size storei_4, storei_4_end-storei_4
storei_8:
    mov tmp, qword ptr [dst + extra]
    mov qword ptr [tmp + 4*idx], rax
storei_8_end:
    .size storei_8, storei_8_end-storei_8

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

# ====> Conversion -> single/double

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
    vcvtsi2ss xmm0, xmm0, eax
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
    vcvtsi2ss xmm0, xmm0, eax
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
    vcvtsi2ss xmm0, xmm0, rax
cvt_u4_f8_end:
    .size cvt_u4_f8, cvt_u4_f8_end-cvt_u4_f8

cvt_i4_f8:
    vcvtsi2sd xmm0, xmm0, eax
cvt_i4_f8_end:
    .size cvt_i4_f8, cvt_i4_f8_end-cvt_i4_f8

cvt_u8_f4:
    vcvtsi2ss xmm0, xmm0, rax
    # Something to be done here
cvt_u8_f4_end:
    .size cvt_u8_f4, cvt_u8_f4_end-cvt_u8_f4

cvt_i8_f4:
    vcvtsi2ss xmm0, xmm0, rax
cvt_i8_f4_end:
    .size cvt_i8_f4, cvt_i8_f4_end-cvt_i8_f4

cvt_u8_f8:
    vcvtsi2ss xmm0, xmm0, rax
    # Something to be done here
cvt_u8_f8_end:
    .size cvt_u8_f8, cvt_u8_f8_end-cvt_u8_f8

cvt_i8_f8:
    vcvtsi2sd xmm0, xmm0, rax
cvt_i8_f8_end:
    .size cvt_i8_f8, cvt_i8_f8_end-cvt_i8_f8
