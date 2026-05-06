#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define in     rcx
#define out    rdx
#define width  r8
#define height r9

#define y      r10
#define x      r11
#define tmp    rbx

// rbx is the GPR scratch (`tmp`) and is callee-saved; xmm6/xmm7 hold the alpha /
// inverse-alpha across the loop body and are callee-saved on Win64 too. Spill
// all three on entry. After `push rbx` the stack is 16-byte aligned, so the
// 32-byte slot keeps the vmovaps targets aligned.
#define cc_prefix \
    push rbx; \
    sub rsp, 32; \
    vmovaps [rsp], xmm6; \
    vmovaps [rsp + 16], xmm7;

#define cc_postfix \
    vmovaps xmm6, [rsp]; \
    vmovaps xmm7, [rsp + 16]; \
    add rsp, 32; \
    pop rbx;

#include "asm_x64.S"
