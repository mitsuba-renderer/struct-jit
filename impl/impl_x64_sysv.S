#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define height rdi
#define width  rsi
#define src    rdx
#define dst    rcx

#define idx    r8
#define y      r9
#define x      r10
#define tmp    r11

#define cc_prefix
#define cc_postfix

#include "impl_x64.S"
