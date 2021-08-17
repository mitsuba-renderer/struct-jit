#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define height rcx
#define width  rdx
#define src    r8
#define dst    r9

#define idx    r10
#define y      r11
#define x      rbx
#define tmp    rdi

#define cc_prefix \
    push rbx; \
    push rdi;

#define cc_postfix \
    pop rdi; \
    pop rbx;

#include "impl_x64.S"
