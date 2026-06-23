; Auto-generated from asm/asm_x64_win.S by asm/gen_masm.py.
; DO NOT EDIT. Regenerate with `make -C asm` after editing the GAS
; sources (asm_x64.S / asm_x64_win.S).
;
; Raw x86-64 (Windows ABI) machine-code templates for the JIT snippet
; library, emitted as data bytes so ml64 can assemble them without a
; GNU assembler. The sentinel displacements/immediates are patched at
; run time by the JIT, exactly as in the GAS original.

OPTION CASEMAP:NONE

_TEXT SEGMENT

PUBLIC main_prefix
PUBLIC main_prefix_end
PUBLIC main_start
PUBLIC main_start_end
PUBLIC main_inc_in
PUBLIC main_inc_in_end
PUBLIC main_inc_out
PUBLIC main_inc_out_end
PUBLIC main_suffix_inner
PUBLIC main_suffix_inner_end
PUBLIC main_suffix_outer
PUBLIC main_suffix_outer_end
PUBLIC main_postfix
PUBLIC main_postfix_end
PUBLIC fail_postfix
PUBLIC fail_postfix_end
PUBLIC load_u1
PUBLIC load_u1_end
PUBLIC load_u2
PUBLIC load_u2_end
PUBLIC load_u4
PUBLIC load_u4_end
PUBLIC load_u8
PUBLIC load_u8_end
PUBLIC load_f2
PUBLIC load_f2_end
PUBLIC load_f4
PUBLIC load_f4_end
PUBLIC load_f8
PUBLIC load_f8_end
PUBLIC store_u1
PUBLIC store_u1_end
PUBLIC store_u2
PUBLIC store_u2_end
PUBLIC store_u4
PUBLIC store_u4_end
PUBLIC store_u8
PUBLIC store_u8_end
PUBLIC store_f2
PUBLIC store_f2_end
PUBLIC store_f4
PUBLIC store_f4_end
PUBLIC store_f8
PUBLIC store_f8_end
PUBLIC bswap_u2
PUBLIC bswap_u2_end
PUBLIC bswap_u4
PUBLIC bswap_u4_end
PUBLIC bswap_u8
PUBLIC bswap_u8_end
PUBLIC mov_u2_f2
PUBLIC mov_u2_f2_end
PUBLIC mov_f2_u2
PUBLIC mov_f2_u2_end
PUBLIC mov_u4_f4
PUBLIC mov_u4_f4_end
PUBLIC mov_f4_u4
PUBLIC mov_f4_u4_end
PUBLIC mov_u8_f8
PUBLIC mov_u8_f8_end
PUBLIC mov_f8_u8
PUBLIC mov_f8_u8_end
PUBLIC cvt_u1_f4
PUBLIC cvt_u1_f4_end
PUBLIC cvt_i1_f4
PUBLIC cvt_i1_f4_end
PUBLIC cvt_u1_f8
PUBLIC cvt_u1_f8_end
PUBLIC cvt_i1_f8
PUBLIC cvt_i1_f8_end
PUBLIC cvt_u2_f4
PUBLIC cvt_u2_f4_end
PUBLIC cvt_i2_f4
PUBLIC cvt_i2_f4_end
PUBLIC cvt_u2_f8
PUBLIC cvt_u2_f8_end
PUBLIC cvt_i2_f8
PUBLIC cvt_i2_f8_end
PUBLIC cvt_u4_f4
PUBLIC cvt_u4_f4_end
PUBLIC cvt_i4_f4
PUBLIC cvt_i4_f4_end
PUBLIC cvt_u4_f8
PUBLIC cvt_u4_f8_end
PUBLIC cvt_i4_f8
PUBLIC cvt_i4_f8_end
PUBLIC cvt_u8_f8
PUBLIC cvt_u8_f8_end
PUBLIC cvt_i8_f8
PUBLIC cvt_i8_f8_end
PUBLIC cvt_u8_f4
PUBLIC cvt_u8_f4_end
PUBLIC cvt_i8_f4
PUBLIC cvt_i8_f4_end
PUBLIC cvt_f4_u1
PUBLIC cvt_f4_i1
PUBLIC cvt_f4_u2
PUBLIC cvt_f4_i2
PUBLIC cvt_f4_i4
PUBLIC cvt_f4_u1_end
PUBLIC cvt_f4_u4
PUBLIC cvt_f4_i8
PUBLIC cvt_f4_u4_end
PUBLIC cvt_f4_u8
PUBLIC cvt_f4_u8_end
PUBLIC cvt_f8_u1
PUBLIC cvt_f8_i1
PUBLIC cvt_f8_u2
PUBLIC cvt_f8_i2
PUBLIC cvt_f8_i4
PUBLIC cvt_f8_u1_end
PUBLIC cvt_f8_u4
PUBLIC cvt_f8_i8
PUBLIC cvt_f8_u4_end
PUBLIC cvt_f8_u8
PUBLIC cvt_f8_u8_end
PUBLIC cvt_f4_f8
PUBLIC cvt_f4_f8_end
PUBLIC cvt_f8_f4
PUBLIC cvt_f8_f4_end
PUBLIC scale_f4
PUBLIC scale_f4_end
PUBLIC scale_f8
PUBLIC scale_f8_end
PUBLIC min_f4
PUBLIC min_f4_end
PUBLIC min_f8
PUBLIC min_f8_end
PUBLIC max_f4
PUBLIC max_f4_end
PUBLIC max_f8
PUBLIC max_f8_end
PUBLIC round_f4
PUBLIC round_f4_end
PUBLIC round_f8
PUBLIC round_f8_end
PUBLIC load_imm
PUBLIC load_imm_end
PUBLIC chk_zload_u1
PUBLIC chk_zload_u1_end
PUBLIC chk_zload_u2
PUBLIC chk_zload_u2_end
PUBLIC load_imm_chk
PUBLIC load_imm_chk_end
PUBLIC chk_ne
PUBLIC chk_ne_end
PUBLIC weight_recip_f4
PUBLIC weight_recip_f4_end
PUBLIC weight_recip_f8
PUBLIC weight_recip_f8_end
PUBLIC weight_scale_f4
PUBLIC weight_scale_f4_end
PUBLIC weight_scale_f8
PUBLIC weight_scale_f8_end
PUBLIC alpha_recip_f4
PUBLIC alpha_recip_f4_end
PUBLIC alpha_recip_f8
PUBLIC alpha_recip_f8_end
PUBLIC alpha_premul_f4
PUBLIC alpha_premul_f4_end
PUBLIC alpha_premul_f8
PUBLIC alpha_premul_f8_end
PUBLIC alpha_unpremul_f4
PUBLIC alpha_unpremul_f4_end
PUBLIC alpha_unpremul_f8
PUBLIC alpha_unpremul_f8_end
PUBLIC blend_init_f4
PUBLIC blend_init_f4_end
PUBLIC blend_init_f8
PUBLIC blend_init_f8_end
PUBLIC blend_add_f4
PUBLIC blend_add_f4_end
PUBLIC blend_add_f8
PUBLIC blend_add_f8_end
PUBLIC blend_fini_f4
PUBLIC blend_fini_f4_end
PUBLIC blend_fini_f8
PUBLIC blend_fini_f8_end
PUBLIC gamma_decode_f4
PUBLIC gamma_decode_f4_end
PUBLIC gamma_encode_f4
PUBLIC gamma_encode_f4_end
PUBLIC gamma_decode_f8
PUBLIC gamma_decode_f8_end
PUBLIC gamma_encode_f8
PUBLIC gamma_encode_f8_end
PUBLIC dither_load_f4
PUBLIC dither_load_f4_end
PUBLIC dither_load_f8
PUBLIC dither_load_f8_end
PUBLIC dither_add_f4
PUBLIC dither_add_f4_end
PUBLIC dither_add_f8
PUBLIC dither_add_f8_end

main_prefix LABEL BYTE
    DB 053h, 048h, 083h, 0ECh, 030h, 0C5h, 0F8h, 029h, 034h, 024h, 0C5h, 0F8h
    DB 029h, 07Ch, 024h, 010h, 0C5h, 078h, 029h, 044h, 024h, 020h, 04Dh, 031h
    DB 0D2h
main_prefix_end LABEL BYTE
main_start LABEL BYTE
    DB 04Dh, 031h, 0DBh
main_start_end LABEL BYTE
main_inc_in LABEL BYTE
    DB 048h, 081h, 0C1h, 078h, 056h, 034h, 012h
main_inc_in_end LABEL BYTE
main_inc_out LABEL BYTE
    DB 048h, 081h, 0C2h, 078h, 056h, 034h, 012h
main_inc_out_end LABEL BYTE
main_suffix_inner LABEL BYTE
    DB 049h, 0FFh, 0C3h, 04Dh, 039h, 0C3h, 00Fh, 085h, 078h, 056h, 034h, 012h
main_suffix_inner_end LABEL BYTE
main_suffix_outer LABEL BYTE
    DB 049h, 0FFh, 0C2h, 04Dh, 039h, 0CAh, 00Fh, 085h, 078h, 056h, 034h, 012h
main_suffix_outer_end LABEL BYTE
main_postfix LABEL BYTE
    DB 0C5h, 0F8h, 028h, 034h, 024h, 0C5h, 0F8h, 028h, 07Ch, 024h, 010h, 0C5h
    DB 078h, 028h, 044h, 024h, 020h, 048h, 083h, 0C4h, 030h, 05Bh, 0B8h, 001h
    DB 000h, 000h, 000h, 0C3h
main_postfix_end LABEL BYTE
fail_postfix LABEL BYTE
    DB 0C5h, 0F8h, 028h, 034h, 024h, 0C5h, 0F8h, 028h, 07Ch, 024h, 010h, 0C5h
    DB 078h, 028h, 044h, 024h, 020h, 048h, 083h, 0C4h, 030h, 05Bh, 031h, 0C0h
    DB 0C3h
fail_postfix_end LABEL BYTE
load_u1 LABEL BYTE
    DB 08Ah, 081h, 078h, 056h, 034h, 012h
load_u1_end LABEL BYTE
load_u2 LABEL BYTE
    DB 066h, 08Bh, 081h, 078h, 056h, 034h, 012h
load_u2_end LABEL BYTE
load_u4 LABEL BYTE
    DB 08Bh, 081h, 078h, 056h, 034h, 012h
load_u4_end LABEL BYTE
load_u8 LABEL BYTE
    DB 048h, 08Bh, 081h, 078h, 056h, 034h, 012h
load_u8_end LABEL BYTE
load_f2 LABEL BYTE
    DB 00Fh, 0B7h, 081h, 078h, 056h, 034h, 012h, 0C5h, 0F9h, 06Eh, 0C0h, 0C4h
    DB 0E2h, 079h, 013h, 0C0h
load_f2_end LABEL BYTE
load_f4 LABEL BYTE
    DB 0C5h, 0FAh, 010h, 081h, 078h, 056h, 034h, 012h
load_f4_end LABEL BYTE
load_f8 LABEL BYTE
    DB 0C5h, 0FBh, 010h, 081h, 078h, 056h, 034h, 012h
load_f8_end LABEL BYTE
store_u1 LABEL BYTE
    DB 088h, 082h, 078h, 056h, 034h, 012h
store_u1_end LABEL BYTE
store_u2 LABEL BYTE
    DB 066h, 089h, 082h, 078h, 056h, 034h, 012h
store_u2_end LABEL BYTE
store_u4 LABEL BYTE
    DB 089h, 082h, 078h, 056h, 034h, 012h
store_u4_end LABEL BYTE
store_u8 LABEL BYTE
    DB 048h, 089h, 082h, 078h, 056h, 034h, 012h
store_u8_end LABEL BYTE
store_f2 LABEL BYTE
    DB 0C4h, 0E3h, 079h, 01Dh, 0C0h, 000h, 0C5h, 0F9h, 07Eh, 0C0h, 066h, 089h
    DB 082h, 078h, 056h, 034h, 012h
store_f2_end LABEL BYTE
store_f4 LABEL BYTE
    DB 0C5h, 0FAh, 011h, 082h, 078h, 056h, 034h, 012h
store_f4_end LABEL BYTE
store_f8 LABEL BYTE
    DB 0C5h, 0FBh, 011h, 082h, 078h, 056h, 034h, 012h
store_f8_end LABEL BYTE
bswap_u2 LABEL BYTE
    DB 086h, 0E0h
bswap_u2_end LABEL BYTE
bswap_u4 LABEL BYTE
    DB 00Fh, 0C8h
bswap_u4_end LABEL BYTE
bswap_u8 LABEL BYTE
    DB 048h, 00Fh, 0C8h
bswap_u8_end LABEL BYTE
mov_u2_f2 LABEL BYTE
    DB 0C5h, 0F9h, 06Eh, 0C0h, 0C4h, 0E2h, 079h, 013h, 0C0h
mov_u2_f2_end LABEL BYTE
mov_f2_u2 LABEL BYTE
    DB 0C4h, 0E3h, 079h, 01Dh, 0C0h, 000h, 0C5h, 0F9h, 07Eh, 0C0h
mov_f2_u2_end LABEL BYTE
mov_u4_f4 LABEL BYTE
    DB 0C5h, 0F9h, 06Eh, 0C0h
mov_u4_f4_end LABEL BYTE
mov_f4_u4 LABEL BYTE
    DB 0C5h, 0F9h, 07Eh, 0C0h
mov_f4_u4_end LABEL BYTE
mov_u8_f8 LABEL BYTE
    DB 0C4h, 0E1h, 0F9h, 06Eh, 0C0h
mov_u8_f8_end LABEL BYTE
mov_f8_u8 LABEL BYTE
    DB 0C4h, 0E1h, 0F9h, 07Eh, 0C0h
mov_f8_u8_end LABEL BYTE
cvt_u1_f4 LABEL BYTE
    DB 00Fh, 0B6h, 0C0h, 0C5h, 0FAh, 02Ah, 0C0h
cvt_u1_f4_end LABEL BYTE
cvt_i1_f4 LABEL BYTE
    DB 00Fh, 0BEh, 0C0h, 0C5h, 0FAh, 02Ah, 0C0h
cvt_i1_f4_end LABEL BYTE
cvt_u1_f8 LABEL BYTE
    DB 00Fh, 0B6h, 0C0h, 0C5h, 0FBh, 02Ah, 0C0h
cvt_u1_f8_end LABEL BYTE
cvt_i1_f8 LABEL BYTE
    DB 00Fh, 0BEh, 0C0h, 0C5h, 0FBh, 02Ah, 0C0h
cvt_i1_f8_end LABEL BYTE
cvt_u2_f4 LABEL BYTE
    DB 00Fh, 0B7h, 0C0h, 0C5h, 0FAh, 02Ah, 0C0h
cvt_u2_f4_end LABEL BYTE
cvt_i2_f4 LABEL BYTE
    DB 00Fh, 0BFh, 0C0h, 0C5h, 0FAh, 02Ah, 0C0h
cvt_i2_f4_end LABEL BYTE
cvt_u2_f8 LABEL BYTE
    DB 00Fh, 0B7h, 0C0h, 0C5h, 0FBh, 02Ah, 0C0h
cvt_u2_f8_end LABEL BYTE
cvt_i2_f8 LABEL BYTE
    DB 00Fh, 0BFh, 0C0h, 0C5h, 0FBh, 02Ah, 0C0h
cvt_i2_f8_end LABEL BYTE
cvt_u4_f4 LABEL BYTE
    DB 0C4h, 0E1h, 0FAh, 02Ah, 0C0h
cvt_u4_f4_end LABEL BYTE
cvt_i4_f4 LABEL BYTE
    DB 0C5h, 0FAh, 02Ah, 0C0h
cvt_i4_f4_end LABEL BYTE
cvt_u4_f8 LABEL BYTE
    DB 0C4h, 0E1h, 0FBh, 02Ah, 0C0h
cvt_u4_f8_end LABEL BYTE
cvt_i4_f8 LABEL BYTE
    DB 0C5h, 0FBh, 02Ah, 0C0h
cvt_i4_f8_end LABEL BYTE
cvt_u8_f8 LABEL BYTE
    DB 048h, 085h, 0C0h, 078h, 007h, 0C4h, 0E1h, 0FBh, 02Ah, 0C0h, 0EBh, 00Ch
    DB 048h, 0D1h, 0E8h, 0C4h, 0E1h, 0FBh, 02Ah, 0C0h, 0C5h, 0FBh, 058h, 0C0h
cvt_u8_f8_end LABEL BYTE
cvt_i8_f8 LABEL BYTE
    DB 0C4h, 0E1h, 0FBh, 02Ah, 0C0h
cvt_i8_f8_end LABEL BYTE
cvt_u8_f4 LABEL BYTE
    DB 048h, 085h, 0C0h, 078h, 007h, 0C4h, 0E1h, 0FAh, 02Ah, 0C0h, 0EBh, 00Ch
    DB 048h, 0D1h, 0E8h, 0C4h, 0E1h, 0FAh, 02Ah, 0C0h, 0C5h, 0FAh, 058h, 0C0h
cvt_u8_f4_end LABEL BYTE
cvt_i8_f4 LABEL BYTE
    DB 0C4h, 0E1h, 0FAh, 02Ah, 0C0h
cvt_i8_f4_end LABEL BYTE
cvt_f4_u1 LABEL BYTE
cvt_f4_i1 LABEL BYTE
cvt_f4_u2 LABEL BYTE
cvt_f4_i2 LABEL BYTE
cvt_f4_i4 LABEL BYTE
    DB 0C5h, 0FAh, 02Dh, 0C0h
cvt_f4_u1_end LABEL BYTE
cvt_f4_u4 LABEL BYTE
cvt_f4_i8 LABEL BYTE
    DB 0C4h, 0E1h, 0FAh, 02Dh, 0C0h
cvt_f4_u4_end LABEL BYTE
cvt_f4_u8 LABEL BYTE
    DB 048h, 0C7h, 0C3h, 000h, 000h, 000h, 05Fh, 0C4h, 0E1h, 0F9h, 06Eh, 0CBh
    DB 0C5h, 0F8h, 02Eh, 0C1h, 073h, 007h, 0C4h, 0E1h, 0FAh, 02Dh, 0C0h, 0EBh
    DB 00Eh, 0C5h, 0FAh, 05Ch, 0C1h, 0C4h, 0E1h, 0FAh, 02Dh, 0C0h, 048h, 00Fh
    DB 0BAh, 0F8h, 03Fh
cvt_f4_u8_end LABEL BYTE
cvt_f8_u1 LABEL BYTE
cvt_f8_i1 LABEL BYTE
cvt_f8_u2 LABEL BYTE
cvt_f8_i2 LABEL BYTE
cvt_f8_i4 LABEL BYTE
    DB 0C5h, 0FBh, 02Dh, 0C0h
cvt_f8_u1_end LABEL BYTE
cvt_f8_u4 LABEL BYTE
cvt_f8_i8 LABEL BYTE
    DB 0C4h, 0E1h, 0FBh, 02Dh, 0C0h
cvt_f8_u4_end LABEL BYTE
cvt_f8_u8 LABEL BYTE
    DB 048h, 0BBh, 000h, 000h, 000h, 000h, 000h, 000h, 0E0h, 043h, 0C4h, 0E1h
    DB 0F9h, 06Eh, 0CBh, 0C5h, 0F9h, 02Eh, 0C1h, 073h, 007h, 0C4h, 0E1h, 0FBh
    DB 02Dh, 0C0h, 0EBh, 00Eh, 0C5h, 0FBh, 05Ch, 0C1h, 0C4h, 0E1h, 0FBh, 02Dh
    DB 0C0h, 048h, 00Fh, 0BAh, 0F8h, 03Fh
cvt_f8_u8_end LABEL BYTE
cvt_f4_f8 LABEL BYTE
    DB 0C5h, 0FAh, 05Ah, 0C0h
cvt_f4_f8_end LABEL BYTE
cvt_f8_f4 LABEL BYTE
    DB 0C5h, 0FBh, 05Ah, 0C0h
cvt_f8_f4_end LABEL BYTE
scale_f4 LABEL BYTE
    DB 0C5h, 0FAh, 059h, 005h, 078h, 056h, 034h, 012h
scale_f4_end LABEL BYTE
scale_f8 LABEL BYTE
    DB 0C5h, 0FBh, 059h, 005h, 078h, 056h, 034h, 012h
scale_f8_end LABEL BYTE
min_f4 LABEL BYTE
    DB 0C5h, 0FAh, 05Dh, 005h, 078h, 056h, 034h, 012h
min_f4_end LABEL BYTE
min_f8 LABEL BYTE
    DB 0C5h, 0FBh, 05Dh, 005h, 078h, 056h, 034h, 012h
min_f8_end LABEL BYTE
max_f4 LABEL BYTE
    DB 0C5h, 0FAh, 05Fh, 005h, 078h, 056h, 034h, 012h
max_f4_end LABEL BYTE
max_f8 LABEL BYTE
    DB 0C5h, 0FBh, 05Fh, 005h, 078h, 056h, 034h, 012h
max_f8_end LABEL BYTE
round_f4 LABEL BYTE
    DB 0C4h, 0E3h, 079h, 00Ah, 0C0h, 000h
round_f4_end LABEL BYTE
round_f8 LABEL BYTE
    DB 0C4h, 0E3h, 079h, 00Bh, 0C0h, 000h
round_f8_end LABEL BYTE
load_imm LABEL BYTE
    DB 048h, 08Bh, 005h, 078h, 056h, 034h, 012h
load_imm_end LABEL BYTE
chk_zload_u1 LABEL BYTE
    DB 00Fh, 0B6h, 081h, 078h, 056h, 034h, 012h
chk_zload_u1_end LABEL BYTE
chk_zload_u2 LABEL BYTE
    DB 00Fh, 0B7h, 081h, 078h, 056h, 034h, 012h
chk_zload_u2_end LABEL BYTE
load_imm_chk LABEL BYTE
    DB 048h, 08Bh, 01Dh, 078h, 056h, 034h, 012h
load_imm_chk_end LABEL BYTE
chk_ne LABEL BYTE
    DB 048h, 039h, 0D8h, 00Fh, 085h, 078h, 056h, 034h, 012h
chk_ne_end LABEL BYTE
weight_recip_f4 LABEL BYTE
    DB 048h, 0C7h, 0C3h, 000h, 000h, 080h, 03Fh, 0C4h, 0E1h, 0F9h, 06Eh, 0D3h
    DB 0C5h, 0F0h, 057h, 0C9h, 0C5h, 0F8h, 02Eh, 0C1h, 07Ah, 002h, 074h, 004h
    DB 0C5h, 0EAh, 05Eh, 0D0h
weight_recip_f4_end LABEL BYTE
weight_recip_f8 LABEL BYTE
    DB 048h, 0BBh, 000h, 000h, 000h, 000h, 000h, 000h, 0F0h, 03Fh, 0C4h, 0E1h
    DB 0F9h, 06Eh, 0D3h, 0C5h, 0F1h, 057h, 0C9h, 0C5h, 0F9h, 02Eh, 0C1h, 07Ah
    DB 002h, 074h, 004h, 0C5h, 0EBh, 05Eh, 0D0h
weight_recip_f8_end LABEL BYTE
weight_scale_f4 LABEL BYTE
    DB 0C5h, 0FAh, 059h, 0C2h
weight_scale_f4_end LABEL BYTE
weight_scale_f8 LABEL BYTE
    DB 0C5h, 0FBh, 059h, 0C2h
weight_scale_f8_end LABEL BYTE
alpha_recip_f4 LABEL BYTE
    DB 0C5h, 0F8h, 028h, 0F0h, 048h, 0C7h, 0C3h, 000h, 000h, 080h, 03Fh, 0C4h
    DB 0E1h, 0F9h, 06Eh, 0FBh, 0C5h, 0F0h, 057h, 0C9h, 0C5h, 0F8h, 02Eh, 0C1h
    DB 07Ah, 002h, 074h, 006h, 0C5h, 0C2h, 05Eh, 0F8h, 0EBh, 004h, 0C5h, 0C0h
    DB 057h, 0FFh
alpha_recip_f4_end LABEL BYTE
alpha_recip_f8 LABEL BYTE
    DB 0C5h, 0F8h, 028h, 0F0h, 048h, 0BBh, 000h, 000h, 000h, 000h, 000h, 000h
    DB 0F0h, 03Fh, 0C4h, 0E1h, 0F9h, 06Eh, 0FBh, 0C5h, 0F1h, 057h, 0C9h, 0C5h
    DB 0F9h, 02Eh, 0C1h, 07Ah, 002h, 074h, 006h, 0C5h, 0C3h, 05Eh, 0F8h, 0EBh
    DB 004h, 0C5h, 0C1h, 057h, 0FFh
alpha_recip_f8_end LABEL BYTE
alpha_premul_f4 LABEL BYTE
    DB 0C5h, 0FAh, 059h, 0C6h
alpha_premul_f4_end LABEL BYTE
alpha_premul_f8 LABEL BYTE
    DB 0C5h, 0FBh, 059h, 0C6h
alpha_premul_f8_end LABEL BYTE
alpha_unpremul_f4 LABEL BYTE
    DB 0C5h, 0FAh, 059h, 0C7h
alpha_unpremul_f4_end LABEL BYTE
alpha_unpremul_f8 LABEL BYTE
    DB 0C5h, 0FBh, 059h, 0C7h
alpha_unpremul_f8_end LABEL BYTE
blend_init_f4 LABEL BYTE
    DB 0C5h, 0F8h, 028h, 0E8h
blend_init_f4_end LABEL BYTE
blend_init_f8 LABEL BYTE
    DB 0C5h, 0F8h, 028h, 0E8h
blend_init_f8_end LABEL BYTE
blend_add_f4 LABEL BYTE
    DB 0C5h, 0D2h, 058h, 0E8h
blend_add_f4_end LABEL BYTE
blend_add_f8 LABEL BYTE
    DB 0C5h, 0D3h, 058h, 0E8h
blend_add_f8_end LABEL BYTE
blend_fini_f4 LABEL BYTE
    DB 0C5h, 0F8h, 028h, 0C5h
blend_fini_f4_end LABEL BYTE
blend_fini_f8 LABEL BYTE
    DB 0C5h, 0F8h, 028h, 0C5h
blend_fini_f8_end LABEL BYTE
gamma_decode_f4 LABEL BYTE
    DB 048h, 08Dh, 01Dh, 078h, 056h, 034h, 012h, 0C5h, 0FAh, 010h, 01Bh, 0C5h
    DB 0F8h, 02Eh, 043h, 004h, 072h, 042h, 0C5h, 0F8h, 028h, 0C8h, 0C5h, 0FAh
    DB 010h, 05Bh, 008h, 0C5h, 0FAh, 010h, 063h, 01Ch, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 00Ch, 0C4h, 0E2h, 071h, 0A9h, 063h, 020h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 010h, 0C4h, 0E2h, 071h, 0A9h, 063h, 024h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 014h, 0C4h, 0E2h, 071h, 0A9h, 063h, 028h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 018h, 0C4h, 0E2h, 071h, 0A9h, 063h, 02Ch, 0C5h, 0E2h, 05Eh, 0DCh
    DB 0C5h, 0FAh, 059h, 0C3h
gamma_decode_f4_end LABEL BYTE
gamma_encode_f4 LABEL BYTE
    DB 048h, 08Dh, 01Dh, 078h, 056h, 034h, 012h, 0C5h, 0FAh, 010h, 01Bh, 0C5h
    DB 0F8h, 02Eh, 043h, 004h, 072h, 04Eh, 0C5h, 0F2h, 051h, 0C8h, 0C5h, 0FAh
    DB 010h, 05Bh, 008h, 0C5h, 0FAh, 010h, 063h, 020h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 00Ch, 0C4h, 0E2h, 071h, 0A9h, 063h, 024h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 010h, 0C4h, 0E2h, 071h, 0A9h, 063h, 028h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 014h, 0C4h, 0E2h, 071h, 0A9h, 063h, 02Ch, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 018h, 0C4h, 0E2h, 071h, 0A9h, 063h, 030h, 0C4h, 0E2h, 071h, 0A9h
    DB 05Bh, 01Ch, 0C4h, 0E2h, 071h, 0A9h, 063h, 034h, 0C5h, 0E2h, 05Eh, 0DCh
    DB 0C5h, 0FAh, 059h, 0C3h
gamma_encode_f4_end LABEL BYTE
gamma_decode_f8 LABEL BYTE
    DB 048h, 08Dh, 01Dh, 078h, 056h, 034h, 012h, 0C5h, 0FBh, 010h, 01Bh, 0C5h
    DB 0F9h, 02Eh, 043h, 008h, 00Fh, 082h, 090h, 000h, 000h, 000h, 0C5h, 0F8h
    DB 028h, 0C8h, 0C5h, 0FBh, 010h, 05Bh, 010h, 0C5h, 0FBh, 010h, 063h, 060h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 018h, 0C4h, 0E2h, 0F1h, 0A9h, 063h, 068h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 020h, 0C4h, 0E2h, 0F1h, 0A9h, 063h, 070h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 028h, 0C4h, 0E2h, 0F1h, 0A9h, 063h, 078h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 030h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 080h
    DB 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 038h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 0A3h, 088h, 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 040h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 090h, 000h, 000h, 000h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 05Bh, 048h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 098h, 000h, 000h, 000h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 050h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 0A0h
    DB 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 058h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 0A3h, 0A8h, 000h, 000h, 000h, 0C5h, 0E3h, 05Eh, 0DCh, 0C5h, 0FBh
    DB 059h, 0C3h
gamma_decode_f8_end LABEL BYTE
gamma_encode_f8 LABEL BYTE
    DB 048h, 08Dh, 01Dh, 078h, 056h, 034h, 012h, 0C5h, 0FBh, 010h, 01Bh, 0C5h
    DB 0F9h, 02Eh, 043h, 008h, 00Fh, 082h, 0A2h, 000h, 000h, 000h, 0C5h, 0F3h
    DB 051h, 0C8h, 0C5h, 0FBh, 010h, 05Bh, 010h, 0C5h, 0FBh, 010h, 063h, 068h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 018h, 0C4h, 0E2h, 0F1h, 0A9h, 063h, 070h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 020h, 0C4h, 0E2h, 0F1h, 0A9h, 063h, 078h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 028h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 080h
    DB 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 030h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 0A3h, 088h, 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 038h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 090h, 000h, 000h, 000h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 05Bh, 040h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 098h, 000h, 000h, 000h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 048h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 0A0h
    DB 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 050h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 0A3h, 0A8h, 000h, 000h, 000h, 0C4h, 0E2h, 0F1h, 0A9h, 05Bh, 058h
    DB 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 0B0h, 000h, 000h, 000h, 0C4h, 0E2h, 0F1h
    DB 0A9h, 05Bh, 060h, 0C4h, 0E2h, 0F1h, 0A9h, 0A3h, 0B8h, 000h, 000h, 000h
    DB 0C5h, 0E3h, 05Eh, 0DCh, 0C5h, 0FBh, 059h, 0C3h
gamma_encode_f8_end LABEL BYTE
dither_load_f4 LABEL BYTE
    DB 04Ch, 089h, 0D8h, 025h, 0FFh, 000h, 000h, 000h, 04Ch, 089h, 0D3h, 048h
    DB 081h, 0E3h, 0FFh, 000h, 000h, 000h, 048h, 0C1h, 0E3h, 008h, 048h, 009h
    DB 0D8h, 048h, 0BBh, 0EFh, 0CDh, 0ABh, 089h, 067h, 045h, 023h, 001h, 0C5h
    DB 07Ah, 010h, 004h, 083h
dither_load_f4_end LABEL BYTE
dither_load_f8 LABEL BYTE
    DB 04Ch, 089h, 0D8h, 025h, 0FFh, 000h, 000h, 000h, 04Ch, 089h, 0D3h, 048h
    DB 081h, 0E3h, 0FFh, 000h, 000h, 000h, 048h, 0C1h, 0E3h, 008h, 048h, 009h
    DB 0D8h, 048h, 0BBh, 0EFh, 0CDh, 0ABh, 089h, 067h, 045h, 023h, 001h, 0C5h
    DB 03Ah, 05Ah, 004h, 083h
dither_load_f8_end LABEL BYTE
dither_add_f4 LABEL BYTE
    DB 0C4h, 0C1h, 07Ah, 058h, 0C0h
dither_add_f4_end LABEL BYTE
dither_add_f8 LABEL BYTE
    DB 0C4h, 0C1h, 07Bh, 058h, 0C0h
dither_add_f8_end LABEL BYTE

_TEXT ENDS
END
