# x86-64 snippet library for Struct-JIT.
#
# GPR roles are macros from the includer; the SysV and Windows mappings differ,
# so snippets must use these macro names (never raw registers) to work on both.
# These roles persist across snippets; everything else (rax, r11, xmm1, xmm3-5)
# is per-snippet scratch.
#   in / out / width / height    kernel arguments
#   y / x                        loop counters
#   tmp                          GPR scratch that survives where a snippet needs it
#   rax                          integer value in flight
#   xmm0                         SIMD value in flight
#   xmm2                         weight reciprocal, live across the loop body
#
# Mappings: SysV  in,out,width,height,y,x,tmp = rdi,rsi,rdx,rcx,r8,r9,r10;
#           Win   = rcx,rdx,r8,r9,r10,r11,rbx (rbx is callee-saved, so cc_prefix/
#           cc_postfix push/pop it). The kernel otherwise touches only
#           caller-saved registers under both ABIs, and makes no calls.

.intel_syntax noprefix
.text

# Hidden visibility: these symbols are private to libstruct-jit. Marking
# them hidden lets intra-.text PC-relative references resolve at link
# time (no GOT), which is required when linked into a shared object. Only
# ELF targets understand the .hidden directive: Mach-O (Apple) and COFF
# (Windows) reject it, and the C++ declarations use asm labels there so the
# raw snippet names still link correctly.
#if !defined(__APPLE__) && !defined(_WIN32)
.hidden main_prefix
.hidden main_prefix_end
.hidden main_start
.hidden main_start_end
.hidden main_inc_in
.hidden main_inc_in_end
.hidden main_inc_out
.hidden main_inc_out_end
.hidden main_suffix_inner
.hidden main_suffix_inner_end
.hidden main_suffix_outer
.hidden main_suffix_outer_end
.hidden main_postfix
.hidden main_postfix_end
.hidden fail_postfix
.hidden fail_postfix_end
.hidden load_u1
.hidden load_u1_end
.hidden load_u2
.hidden load_u2_end
.hidden load_u4
.hidden load_u4_end
.hidden load_u8
.hidden load_u8_end
.hidden load_f2
.hidden load_f2_end
.hidden load_f4
.hidden load_f4_end
.hidden load_f8
.hidden load_f8_end
.hidden store_u1
.hidden store_u1_end
.hidden store_u2
.hidden store_u2_end
.hidden store_u4
.hidden store_u4_end
.hidden store_u8
.hidden store_u8_end
.hidden store_f2
.hidden store_f2_end
.hidden store_f4
.hidden store_f4_end
.hidden store_f8
.hidden store_f8_end
.hidden bswap_u2
.hidden bswap_u2_end
.hidden bswap_u4
.hidden bswap_u4_end
.hidden bswap_u8
.hidden bswap_u8_end
.hidden mov_u2_f2
.hidden mov_u2_f2_end
.hidden mov_f2_u2
.hidden mov_f2_u2_end
.hidden mov_u4_f4
.hidden mov_u4_f4_end
.hidden mov_f4_u4
.hidden mov_f4_u4_end
.hidden mov_u8_f8
.hidden mov_u8_f8_end
.hidden mov_f8_u8
.hidden mov_f8_u8_end
.hidden cvt_u1_f4
.hidden cvt_u1_f4_end
.hidden cvt_i1_f4
.hidden cvt_i1_f4_end
.hidden cvt_u1_f8
.hidden cvt_u1_f8_end
.hidden cvt_i1_f8
.hidden cvt_i1_f8_end
.hidden cvt_u2_f4
.hidden cvt_u2_f4_end
.hidden cvt_i2_f4
.hidden cvt_i2_f4_end
.hidden cvt_u2_f8
.hidden cvt_u2_f8_end
.hidden cvt_i2_f8
.hidden cvt_i2_f8_end
.hidden cvt_u4_f4
.hidden cvt_u4_f4_end
.hidden cvt_i4_f4
.hidden cvt_i4_f4_end
.hidden cvt_u4_f8
.hidden cvt_u4_f8_end
.hidden cvt_i4_f8
.hidden cvt_i4_f8_end
.hidden cvt_u8_f8
.hidden cvt_u8_f8_end
.hidden cvt_i8_f8
.hidden cvt_i8_f8_end
.hidden cvt_u8_f4
.hidden cvt_u8_f4_end
.hidden cvt_i8_f4
.hidden cvt_i8_f4_end
.hidden cvt_f4_u1
.hidden cvt_f4_i1
.hidden cvt_f4_u2
.hidden cvt_f4_i2
.hidden cvt_f4_i4
.hidden cvt_f4_u1_end
.hidden cvt_f4_u4
.hidden cvt_f4_i8
.hidden cvt_f4_u4_end
.hidden cvt_f4_u8
.hidden cvt_f4_u8_end
.hidden cvt_f8_u1
.hidden cvt_f8_i1
.hidden cvt_f8_u2
.hidden cvt_f8_i2
.hidden cvt_f8_i4
.hidden cvt_f8_u1_end
.hidden cvt_f8_u4
.hidden cvt_f8_i8
.hidden cvt_f8_u4_end
.hidden cvt_f8_u8
.hidden cvt_f8_u8_end
.hidden cvt_f4_f8
.hidden cvt_f4_f8_end
.hidden cvt_f8_f4
.hidden cvt_f8_f4_end
.hidden scale_f4
.hidden scale_f4_end
.hidden scale_f8
.hidden scale_f8_end
.hidden min_f4
.hidden min_f4_end
.hidden min_f8
.hidden min_f8_end
.hidden max_f4
.hidden max_f4_end
.hidden max_f8
.hidden max_f8_end
.hidden round_f4
.hidden round_f4_end
.hidden round_f8
.hidden round_f8_end
.hidden load_imm
.hidden load_imm_end
.hidden chk_zload_u1
.hidden chk_zload_u1_end
.hidden chk_zload_u2
.hidden chk_zload_u2_end
.hidden load_imm_chk
.hidden load_imm_chk_end
.hidden chk_ne
.hidden chk_ne_end
.hidden weight_recip_f4
.hidden weight_recip_f4_end
.hidden weight_recip_f8
.hidden weight_recip_f8_end
.hidden weight_scale_f4
.hidden weight_scale_f4_end
.hidden weight_scale_f8
.hidden weight_scale_f8_end
.hidden alpha_recip_f4
.hidden alpha_recip_f4_end
.hidden alpha_recip_f8
.hidden alpha_recip_f8_end
.hidden alpha_premul_f4
.hidden alpha_premul_f4_end
.hidden alpha_premul_f8
.hidden alpha_premul_f8_end
.hidden alpha_unpremul_f4
.hidden alpha_unpremul_f4_end
.hidden alpha_unpremul_f8
.hidden alpha_unpremul_f8_end
.hidden blend_init_f4
.hidden blend_init_f4_end
.hidden blend_init_f8
.hidden blend_init_f8_end
.hidden blend_add_f4
.hidden blend_add_f4_end
.hidden blend_add_f8
.hidden blend_add_f8_end
.hidden blend_fini_f4
.hidden blend_fini_f4_end
.hidden blend_fini_f8
.hidden blend_fini_f8_end
.hidden gamma_decode_f4
.hidden gamma_decode_f4_end
.hidden gamma_encode_f4
.hidden gamma_encode_f4_end
.hidden gamma_decode_f8
.hidden gamma_decode_f8_end
.hidden gamma_encode_f8
.hidden gamma_encode_f8_end
.hidden dither_f4
.hidden dither_f4_end
.hidden dither_f8
.hidden dither_f8_end
#endif

.globl main_prefix
.globl main_prefix_end
.globl main_start
.globl main_start_end
.globl main_inc_in
.globl main_inc_in_end
.globl main_inc_out
.globl main_inc_out_end
.globl main_suffix_inner
.globl main_suffix_inner_end
.globl main_suffix_outer
.globl main_suffix_outer_end
.globl main_postfix
.globl main_postfix_end
.globl fail_postfix
.globl fail_postfix_end

.globl load_u1
.globl load_u1_end
.globl load_u2
.globl load_u2_end
.globl load_u4
.globl load_u4_end
.globl load_u8
.globl load_u8_end
.globl load_f2
.globl load_f2_end
.globl load_f4
.globl load_f4_end
.globl load_f8
.globl load_f8_end

.globl store_u1
.globl store_u1_end
.globl store_u2
.globl store_u2_end
.globl store_u4
.globl store_u4_end
.globl store_u8
.globl store_u8_end
.globl store_f2
.globl store_f2_end
.globl store_f4
.globl store_f4_end
.globl store_f8
.globl store_f8_end

.globl bswap_u2
.globl bswap_u2_end
.globl bswap_u4
.globl bswap_u4_end
.globl bswap_u8
.globl bswap_u8_end

.globl mov_u2_f2
.globl mov_u2_f2_end
.globl mov_f2_u2
.globl mov_f2_u2_end
.globl mov_u4_f4
.globl mov_u4_f4_end
.globl mov_f4_u4
.globl mov_f4_u4_end
.globl mov_u8_f8
.globl mov_u8_f8_end
.globl mov_f8_u8
.globl mov_f8_u8_end

.globl cvt_u1_f4
.globl cvt_u1_f4_end
.globl cvt_i1_f4
.globl cvt_i1_f4_end
.globl cvt_u1_f8
.globl cvt_u1_f8_end
.globl cvt_i1_f8
.globl cvt_i1_f8_end
.globl cvt_u2_f4
.globl cvt_u2_f4_end
.globl cvt_i2_f4
.globl cvt_i2_f4_end
.globl cvt_u2_f8
.globl cvt_u2_f8_end
.globl cvt_i2_f8
.globl cvt_i2_f8_end
.globl cvt_u4_f4
.globl cvt_u4_f4_end
.globl cvt_i4_f4
.globl cvt_i4_f4_end
.globl cvt_u4_f8
.globl cvt_u4_f8_end
.globl cvt_i4_f8
.globl cvt_i4_f8_end
.globl cvt_u8_f8
.globl cvt_u8_f8_end
.globl cvt_i8_f8
.globl cvt_i8_f8_end
.globl cvt_u8_f4
.globl cvt_u8_f4_end
.globl cvt_i8_f4
.globl cvt_i8_f4_end

.globl cvt_f4_u1
.globl cvt_f4_i1
.globl cvt_f4_u2
.globl cvt_f4_i2
.globl cvt_f4_i4
.globl cvt_f4_u1_end
.globl cvt_f4_u4
.globl cvt_f4_i8
.globl cvt_f4_u4_end
.globl cvt_f4_u8
.globl cvt_f4_u8_end
.globl cvt_f8_u1
.globl cvt_f8_i1
.globl cvt_f8_u2
.globl cvt_f8_i2
.globl cvt_f8_i4
.globl cvt_f8_u1_end
.globl cvt_f8_u4
.globl cvt_f8_i8
.globl cvt_f8_u4_end
.globl cvt_f8_u8
.globl cvt_f8_u8_end

.globl cvt_f4_f8
.globl cvt_f4_f8_end
.globl cvt_f8_f4
.globl cvt_f8_f4_end

.globl scale_f4
.globl scale_f4_end
.globl scale_f8
.globl scale_f8_end
.globl min_f4
.globl min_f4_end
.globl min_f8
.globl min_f8_end
.globl max_f4
.globl max_f4_end
.globl max_f8
.globl max_f8_end
.globl round_f4
.globl round_f4_end
.globl round_f8
.globl round_f8_end
.globl load_imm
.globl load_imm_end
.globl chk_zload_u1
.globl chk_zload_u1_end
.globl chk_zload_u2
.globl chk_zload_u2_end
.globl load_imm_chk
.globl load_imm_chk_end
.globl chk_ne
.globl chk_ne_end

.globl weight_recip_f4
.globl weight_recip_f4_end
.globl weight_recip_f8
.globl weight_recip_f8_end
.globl weight_scale_f4
.globl weight_scale_f4_end
.globl weight_scale_f8
.globl weight_scale_f8_end

.globl alpha_recip_f4
.globl alpha_recip_f4_end
.globl alpha_recip_f8
.globl alpha_recip_f8_end
.globl alpha_premul_f4
.globl alpha_premul_f4_end
.globl alpha_premul_f8
.globl alpha_premul_f8_end
.globl alpha_unpremul_f4
.globl alpha_unpremul_f4_end
.globl alpha_unpremul_f8
.globl alpha_unpremul_f8_end

.globl blend_init_f4
.globl blend_init_f4_end
.globl blend_init_f8
.globl blend_init_f8_end
.globl blend_add_f4
.globl blend_add_f4_end
.globl blend_add_f8
.globl blend_add_f8_end
.globl blend_fini_f4
.globl blend_fini_f4_end
.globl blend_fini_f8
.globl blend_fini_f8_end

.globl gamma_decode_f4
.globl gamma_decode_f4_end
.globl gamma_encode_f4
.globl gamma_encode_f4_end
.globl gamma_decode_f8
.globl gamma_decode_f8_end
.globl gamma_encode_f8
.globl gamma_encode_f8_end

.globl dither_f4
.globl dither_f4_end
.globl dither_f8
.globl dither_f8_end

# void convert(size_t height, size_t width, const void *in, void *out) {
#     for (size_t y = 0; y < height; ++y) {
#         for (size_t x = 0; x < width; ++x) {
#             for (each field) {
#             }
#         }
#     }
# }

# [x] Gamma-correct, gamma-uncorrect
# [x] Normalize, unnormlalize
# [x] Weight division
# [x] Dithering
# [x] Float -> int saturation (clamp)
# [x] Default substitution
# [x] Check / assert
# [x] Alpha premult, unpremult
# [x] Blend
# [x] Endianness

# Loop body setup
main_prefix:
    cc_prefix
    xor y, y
main_prefix_end:

main_start: # Outer loop start
    xor x, x
main_start_end:

# << Loop body goes here >>

main_inc_in:
    add in, extra
main_inc_in_end:

main_inc_out:
    add out, extra
main_inc_out_end:

main_suffix_inner:
    inc x
    cmp x, width
    jne main_suffix_inner_end + extra # -> loop body
main_suffix_inner_end:

main_suffix_outer:
    inc y
    cmp y, height
    jne main_suffix_outer_end + extra # -> main_start
main_suffix_outer_end:

main_postfix:
    cc_postfix
    mov eax, 1
    ret
main_postfix_end:

# Failure epilogue: a 'Check'-flagged field did not match its expected value.
# Restore callee-saved state (Windows) and return false. Branched to from chk_ne.
fail_postfix:
    cc_postfix
    xor eax, eax
    ret
fail_postfix_end:

# ====> Load direct

load_u1:
    mov al, byte ptr [in + extra]
load_u1_end:

load_u2:
    mov ax, word ptr [in + extra]
load_u2_end:

load_u4:
    mov eax, dword ptr [in + extra]
load_u4_end:

load_u8:
    mov rax, qword ptr [in + extra]
load_u8_end:

load_f2:
    movzx eax, word ptr [in + extra]
    vmovd xmm0, eax
    vcvtph2ps xmm0, xmm0
load_f2_end:

load_f4:
    vmovss xmm0, dword ptr [in + extra]
load_f4_end:

load_f8:
    vmovsd xmm0, qword ptr [in + extra]
load_f8_end:

# ====> Store direct

store_u1:
    mov byte ptr [out + extra], al
store_u1_end:

store_u2:
    mov word ptr [out + extra], ax
store_u2_end:

store_u4:
    mov dword ptr [out + extra], eax
store_u4_end:

store_u8:
    mov qword ptr [out + extra], rax
store_u8_end:

store_f2:
    vcvtps2ph xmm0, xmm0, 0
    vmovd eax, xmm0
    mov word ptr [out + extra], ax
store_f2_end:

store_f4:
    vmovss dword ptr [out + extra], xmm0
store_f4_end:

store_f8:
    vmovsd qword ptr [out + extra], xmm0
store_f8_end:

# ====> Endianness swap, register exchange

bswap_u2:
    xchg ah, al
bswap_u2_end:

bswap_u4:
    bswap eax
bswap_u4_end:

bswap_u8:
    bswap rax
bswap_u8_end:

mov_u2_f2:
    vmovd xmm0, eax
    vcvtph2ps xmm0, xmm0
mov_u2_f2_end:

mov_f2_u2:
    vcvtps2ph xmm0, xmm0, 0
    vmovd eax, xmm0
mov_f2_u2_end:

mov_u4_f4:
    vmovd xmm0, eax
mov_u4_f4_end:

mov_f4_u4:
    vmovd eax, xmm0
mov_f4_u4_end:

mov_u8_f8:
    vmovq xmm0, rax
mov_u8_f8_end:

mov_f8_u8:
    vmovq rax, xmm0
mov_f8_u8_end:

# ====> Integer -> Float conversion

cvt_u1_f4:
    movzx eax, al
    vcvtsi2ss xmm0, xmm0, eax
cvt_u1_f4_end:

cvt_i1_f4:
    movsx eax, al
    vcvtsi2ss xmm0, xmm0, eax
cvt_i1_f4_end:

cvt_u1_f8:
    movzx eax, al
    vcvtsi2sd xmm0, xmm0, eax
cvt_u1_f8_end:

cvt_i1_f8:
    movsx eax, al
    vcvtsi2sd xmm0, xmm0, eax
cvt_i1_f8_end:

cvt_u2_f4:
    movzx eax, ax
    vcvtsi2ss xmm0, xmm0, eax
cvt_u2_f4_end:

cvt_i2_f4:
    movsx eax, ax
    vcvtsi2ss xmm0, xmm0, eax
cvt_i2_f4_end:

cvt_u2_f8:
    movzx eax, ax
    vcvtsi2sd xmm0, xmm0, eax
cvt_u2_f8_end:

cvt_i2_f8:
    movsx eax, ax
    vcvtsi2sd xmm0, xmm0, eax
cvt_i2_f8_end:

cvt_u4_f4:
    vcvtsi2ss xmm0, xmm0, rax
cvt_u4_f4_end:

cvt_i4_f4:
    vcvtsi2ss xmm0, xmm0, eax
cvt_i4_f4_end:

cvt_u4_f8:
    vcvtsi2sd xmm0, xmm0, rax
cvt_u4_f8_end:

cvt_i4_f8:
    vcvtsi2sd xmm0, xmm0, eax
cvt_i4_f8_end:

cvt_u8_f8:
    test rax, rax
    js 1f
    vcvtsi2sd xmm0, xmm0, rax
    jmp 2f
1:  shr rax
    vcvtsi2sd xmm0, xmm0, rax
    vaddsd xmm0, xmm0, xmm0
2:
cvt_u8_f8_end:

cvt_i8_f8:
    vcvtsi2sd xmm0, xmm0, rax
cvt_i8_f8_end:

cvt_u8_f4:
    test rax, rax
    js 1f
    vcvtsi2ss xmm0, xmm0, rax
    jmp 2f
1:  shr rax
    vcvtsi2ss xmm0, xmm0, rax
    vaddss xmm0, xmm0, xmm0
2:
cvt_u8_f4_end:

cvt_i8_f4:
    vcvtsi2ss xmm0, xmm0, rax
cvt_i8_f4_end:

# ====> Float -> Integer conversion

cvt_f4_u1:
cvt_f4_i1:
cvt_f4_u2:
cvt_f4_i2:
cvt_f4_i4:
    vcvtss2si eax, xmm0
cvt_f4_u1_end:

cvt_f4_u4:
cvt_f4_i8:
    vcvtss2si rax, xmm0
cvt_f4_u4_end:

cvt_f4_u8:
    mov tmp, 0x5f000000 # 9.22337e+18
    vmovd xmm1, tmp
    vucomiss xmm0, xmm1
    jnb 1f
    vcvtss2si rax, xmm0
    jmp 2f
1:  vsubss xmm0, xmm0, xmm1
    vcvtss2si rax, xmm0
    btc rax, 63
2:
cvt_f4_u8_end:

cvt_f8_u1:
cvt_f8_i1:
cvt_f8_u2:
cvt_f8_i2:
cvt_f8_i4:
    vcvtsd2si eax, xmm0
cvt_f8_u1_end:

cvt_f8_u4:
cvt_f8_i8:
    vcvtsd2si rax, xmm0
cvt_f8_u4_end:

cvt_f8_u8:
    mov tmp, 0x43e0000000000000 # 9.223372036854776e+18
    vmovq xmm1, tmp
    vucomisd xmm0, xmm1
    jnb 1f
    vcvtsd2si rax, xmm0
    jmp 2f
1:  vsubsd xmm0, xmm0, xmm1
    vcvtsd2si rax, xmm0
    btc rax, 63
2:
cvt_f8_u8_end:

# ====> Float -> Float conversion

cvt_f4_f8:
    vcvtss2sd xmm0, xmm0, xmm0
cvt_f4_f8_end:

cvt_f8_f4:
    vcvtsd2ss xmm0, xmm0, xmm0
cvt_f8_f4_end:

# ====> Simple transformations

scale_f4:
    vmulss xmm0, xmm0, dword ptr [rip+extra]
scale_f4_end:

scale_f8:
    vmulsd xmm0, xmm0, qword ptr [rip+extra]
scale_f8_end:

min_f4:
    vminss xmm0, xmm0, dword ptr [rip+extra]
min_f4_end:

min_f8:
    vminsd xmm0, xmm0, qword ptr [rip+extra]
min_f8_end:

max_f4:
    vmaxss xmm0, xmm0, dword ptr [rip+extra]
max_f4_end:

max_f8:
    vmaxsd xmm0, xmm0, qword ptr [rip+extra]
max_f8_end:

round_f4:
    vroundss xmm0, xmm0, xmm0, 0
round_f4_end:

round_f8:
    vroundsd xmm0, xmm0, xmm0, 0
round_f8_end:

# ====> Default value materialization
#
# Load a field's raw default bits (zero-extended to 8 bytes) from the tail
# literal pool into rax. The matching integer store (store_u1/u2/u4/u8) commits
# only the low bytes of the destination type. RIP-relative, patched like scale.

load_imm:
    mov rax, qword ptr [rip+extra]
load_imm_end:

# ====> 'Check' (assert) handling
#
# Verify that a flagged input field holds an expected value. The raw field bytes
# are loaded zero-extended into rax (chk_zload_*), byte-swapped if the input is
# big-endian (bswap_*), and the expected bits are loaded into `tmp` from the
# literal pool (load_imm_chk). chk_ne then compares and branches to fail_postfix
# (a forward jump patched by the codegen) on mismatch.

chk_zload_u1:
    movzx eax, byte ptr [in + extra]
chk_zload_u1_end:

chk_zload_u2:
    movzx eax, word ptr [in + extra]
chk_zload_u2_end:

load_imm_chk:
    mov tmp, qword ptr [rip+extra]
load_imm_chk_end:

chk_ne:
    cmp rax, tmp
    jne chk_ne_end + extra    # b.ne fail (rel32 sentinel, patched to fail_postfix)
chk_ne_end:

# ====> Weight handling

weight_recip_f4:
    mov tmp, 0x3f800000 # 1.0
    vmovd xmm2, tmp
    vxorps xmm1, xmm1, xmm1
    vucomiss xmm0, xmm1
    jp 1f # weight == NaN (propagate)
    je 2f # weight == 0 (skip division)
1:  vdivss xmm2, xmm2, xmm0
2:
weight_recip_f4_end:

weight_recip_f8:
    mov tmp, 0x3ff0000000000000 # 1.0
    vmovq xmm2, tmp
    vxorpd xmm1, xmm1, xmm1
    vucomisd xmm0, xmm1
    jp 1f # weight == NaN (propagate)
    je 2f # weight == 0 (skip division)
1:  vdivsd xmm2, xmm2, xmm0
2:
weight_recip_f8_end:

weight_scale_f4:
    vmulss xmm0, xmm0, xmm2
weight_scale_f4_end:

weight_scale_f8:
    vmulsd xmm0, xmm0, xmm2
weight_scale_f8_end:

# ====> Alpha (un)premultiplication
#
# alpha_recip takes the linearized source alpha in xmm0 and stashes alpha in
# xmm6 and 1/alpha in xmm7 (0 if alpha == 0; a NaN alpha yields a NaN
# reciprocal). xmm6/xmm7 persist across the loop body; under the Windows ABI they
# are callee-saved, so cc_prefix/cc_postfix spill them. alpha_premul /
# alpha_unpremul multiply the working value by alpha / inv_alpha.

alpha_recip_f4:
    vmovaps xmm6, xmm0           # alpha = a
    mov tmp, 0x3f800000          # 1.0
    vmovd xmm7, tmp
    vxorps xmm1, xmm1, xmm1
    vucomiss xmm0, xmm1
    jp 1f                        # alpha == NaN -> 1/NaN (propagate)
    je 2f                        # alpha == 0   -> inv = 0
1:  vdivss xmm7, xmm7, xmm0
    jmp 3f
2:  vxorps xmm7, xmm7, xmm7
3:
alpha_recip_f4_end:

alpha_recip_f8:
    vmovaps xmm6, xmm0
    mov tmp, 0x3ff0000000000000  # 1.0
    vmovq xmm7, tmp
    vxorpd xmm1, xmm1, xmm1
    vucomisd xmm0, xmm1
    jp 1f
    je 2f
1:  vdivsd xmm7, xmm7, xmm0
    jmp 3f
2:  vxorpd xmm7, xmm7, xmm7
3:
alpha_recip_f8_end:

alpha_premul_f4:
    vmulss xmm0, xmm0, xmm6
alpha_premul_f4_end:

alpha_premul_f8:
    vmulsd xmm0, xmm0, xmm6
alpha_premul_f8_end:

alpha_unpremul_f4:
    vmulss xmm0, xmm0, xmm7
alpha_unpremul_f4_end:

alpha_unpremul_f8:
    vmulsd xmm0, xmm0, xmm7
alpha_unpremul_f8_end:

# ====> Blend accumulation
#
# A blended output sums several weighted, linearized source terms in the working
# precision. blend_init seeds the accumulator (xmm5) with the first weighted term
# (already scaled by its coefficient in xmm0); blend_add folds in each subsequent
# term; blend_fini moves the accumulator back into xmm0 for the normal store
# path. xmm5 is scratch elsewhere (dither) but is not touched by the load /
# convert / scale / gamma snippets between terms.

blend_init_f4:
    vmovaps xmm5, xmm0
blend_init_f4_end:

blend_init_f8:
    vmovaps xmm5, xmm0
blend_init_f8_end:

blend_add_f4:
    vaddss xmm5, xmm5, xmm0
blend_add_f4_end:

blend_add_f8:
    vaddsd xmm5, xmm5, xmm0
blend_add_f8_end:

blend_fini_f4:
    vmovaps xmm0, xmm5
blend_fini_f4_end:

blend_fini_f8:
    vmovaps xmm0, xmm5
blend_fini_f8_end:

# ====> Gamma (sRGB) correction
#
# Rational polynomial (Mitsuba StructConverter::gamma()), no calls. x in xmm0
# (preserved), result in xmm0; xmm1 = y, xmm3 = a (num/ratio/multiplier),
# xmm4 = den. Coefficients live in the shared tail constant pool: `lea tmp,
# [rip + extra]` is a placeholder whose rel32 the codegen patches to the table
# (one fixup per snippet), and the per-coefficient `[tmp + n]` offsets follow the
# flat gamma_coeffs.h layout [scale, threshold, num[0..k], den[0..k]]. `tmp` is
# dead here (the integer value is already in xmm0). xmm2 (weight reciprocal) is
# left untouched so gamma and weight compose. result = ratio(y) * x, with the
# linear segment scale*x for x < threshold.

# Haswell+ (FMA): num = num*y + coeff in one op (matches Mitsuba's fmadd213).
.macro GMACC_X4 no, do
    vfmadd213ss xmm3, xmm1, dword ptr [tmp + \no]
    vfmadd213ss xmm4, xmm1, dword ptr [tmp + \do]
.endm
.macro GMACC_X8 no, do
    vfmadd213sd xmm3, xmm1, qword ptr [tmp + \no]
    vfmadd213sd xmm4, xmm1, qword ptr [tmp + \do]
.endm

gamma_decode_f4:                         # sRGB -> linear (n = 5, y = x)
    lea tmp, [rip + extra]               # table base (rel32 patched to pool)
    vmovss xmm3, dword ptr [tmp + 0]     # a = 1/12.92
    vucomiss xmm0, dword ptr [tmp + 4]   # x vs threshold
    jb 0f
    vmovaps xmm1, xmm0                   # y = x
    vmovss xmm3, dword ptr [tmp + 8]     # num[0]
    vmovss xmm4, dword ptr [tmp + 28]    # den[0]
    GMACC_X4 12, 32
    GMACC_X4 16, 36
    GMACC_X4 20, 40
    GMACC_X4 24, 44
    vdivss xmm3, xmm3, xmm4
0:  vmulss xmm0, xmm0, xmm3
gamma_decode_f4_end:

gamma_encode_f4:                         # linear -> sRGB (n = 6, y = sqrt(x))
    lea tmp, [rip + extra]               # table base (rel32 patched to pool)
    vmovss xmm3, dword ptr [tmp + 0]     # a = 12.92
    vucomiss xmm0, dword ptr [tmp + 4]
    jb 0f
    vsqrtss xmm1, xmm1, xmm0             # y = sqrt(x)
    vmovss xmm3, dword ptr [tmp + 8]
    vmovss xmm4, dword ptr [tmp + 32]
    GMACC_X4 12, 36
    GMACC_X4 16, 40
    GMACC_X4 20, 44
    GMACC_X4 24, 48
    GMACC_X4 28, 52
    vdivss xmm3, xmm3, xmm4
0:  vmulss xmm0, xmm0, xmm3
gamma_encode_f4_end:

gamma_decode_f8:                         # sRGB -> linear (n = 10, y = x)
    lea tmp, [rip + extra]               # table base (rel32 patched to pool)
    vmovsd xmm3, qword ptr [tmp + 0]     # a = 1/12.92
    vucomisd xmm0, qword ptr [tmp + 8]
    jb 0f
    vmovaps xmm1, xmm0                   # y = x
    vmovsd xmm3, qword ptr [tmp + 16]    # num[0]
    vmovsd xmm4, qword ptr [tmp + 96]    # den[0]
    GMACC_X8 24, 104
    GMACC_X8 32, 112
    GMACC_X8 40, 120
    GMACC_X8 48, 128
    GMACC_X8 56, 136
    GMACC_X8 64, 144
    GMACC_X8 72, 152
    GMACC_X8 80, 160
    GMACC_X8 88, 168
    vdivsd xmm3, xmm3, xmm4
0:  vmulsd xmm0, xmm0, xmm3
gamma_decode_f8_end:

gamma_encode_f8:                         # linear -> sRGB (n = 11, y = sqrt(x))
    lea tmp, [rip + extra]               # table base (rel32 patched to pool)
    vmovsd xmm3, qword ptr [tmp + 0]     # a = 12.92
    vucomisd xmm0, qword ptr [tmp + 8]
    jb 0f
    vsqrtsd xmm1, xmm1, xmm0             # y = sqrt(x)
    vmovsd xmm3, qword ptr [tmp + 16]
    vmovsd xmm4, qword ptr [tmp + 104]
    GMACC_X8 24, 112
    GMACC_X8 32, 120
    GMACC_X8 40, 128
    GMACC_X8 48, 136
    GMACC_X8 56, 144
    GMACC_X8 64, 152
    GMACC_X8 72, 160
    GMACC_X8 80, 168
    GMACC_X8 88, 176
    GMACC_X8 96, 184
    vdivsd xmm3, xmm3, xmm4
0:  vmulsd xmm0, xmm0, xmm3
gamma_encode_f8_end:

# ====> Ordered dithering
#
# Add dither_matrix256[(y%256)*256 + (x%256)] to the working value before
# quantization. y = r8, x = r9; rax/r11/r10 are scratch (rcx is `height`, so it
# must NOT be touched). The table base is baked into the movabs immediate, which
# the codegen patches by replacing the sentinel.

dither_f4:
    mov rax, x
    and eax, 0xff                        # x % 256
    mov tmp, y
    and tmp, 0xff                        # y % 256
    shl tmp, 8
    or rax, tmp                          # idx = (y%256)*256 + (x%256)
    movabs tmp, 0x0123456789ABCDEF       # table base (patched)
    vaddss xmm0, xmm0, dword ptr [tmp + rax*4]
dither_f4_end:

dither_f8:
    mov rax, x
    and eax, 0xff
    mov tmp, y
    and tmp, 0xff
    shl tmp, 8
    or rax, tmp
    movabs tmp, 0x0123456789ABCDEF
    vcvtss2sd xmm5, xmm5, dword ptr [tmp + rax*4]   # table value f32 -> f64
    vaddsd xmm0, xmm0, xmm5
dither_f8_end:
