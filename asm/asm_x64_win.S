#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define height rcx
#define width  rdx
#define src    r8
#define dst    r9

#define y      r10
#define x      r11
#define tmp    rbx

#define cc_prefix \
    push rbx;

#define cc_postfix \
    pop rbx;

#include "asm_x64.S"
