#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define in     rcx
#define out    rdx
#define width  r8
#define height r9

#define y      r10
#define x      r11
#define tmp    rbx

// tmp (rbx) and the loop-body registers xmm6/xmm7 (alpha) and xmm8 (dither) are
// callee-saved on Win64; spill them on entry. `push rbx` 16-aligns rsp and 48 is
// a multiple of 16, so the vmovaps targets stay aligned.
#define cc_prefix \
    push rbx; \
    sub rsp, 48; \
    vmovaps [rsp], xmm6; \
    vmovaps [rsp + 16], xmm7; \
    vmovaps [rsp + 32], xmm8;

#define cc_postfix \
    vmovaps xmm6, [rsp]; \
    vmovaps xmm7, [rsp + 16]; \
    vmovaps xmm8, [rsp + 32]; \
    add rsp, 48; \
    pop rbx;

#include "asm_x64.S"
