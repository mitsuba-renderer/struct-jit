#define extra 0x12345678 // Dummy value in assembly snippets that will be replaced

#define in     rdi
#define out    rsi
#define width  rdx
#define height rcx

#define y      r8
#define x      r9
#define tmp    r10

#define cc_prefix
#define cc_postfix

#include "asm_x64.S"

// The snippets never require an executable stack; mark it so explicitly on ELF
// to avoid the linker defaulting libstruct-jit to an executable stack.
#if !defined(__APPLE__)
.section .note.GNU-stack,"",@progbits
#endif
