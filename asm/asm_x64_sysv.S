#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define height rdi
#define width  rsi
#define src    rdx
#define dst    rcx

#define y      r8
#define x      r9
#define tmp    r10

#define cc_prefix
#define cc_postfix

#include "asm_x64.S"
