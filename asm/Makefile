# One-time (re)generation of the MASM snippet library used by MSVC builds.
#
# This is NOT part of the CMake build. The generated asm_x64_win.asm is checked
# in and compiled as-is by ml64 on Windows; ordinary builds never invoke this.
# Re-run it only after editing the GNU-assembler sources (asm_x64.S /
# asm_x64_win.S), then commit the regenerated asm_x64_win.asm.
#
#   make            # regenerate asm_x64_win.asm
#   make clean      # remove the generated file
#
# Requires clang (cross-assembles to a temporary ELF object) and python3.
# No binutils needed; runs on Linux or macOS.

CLANG  ?= clang
PYTHON ?= python3

GENERATED := asm_x64_win.asm

all: $(GENERATED)

$(GENERATED): asm_x64_win.S asm_x64.S gen_masm.py
	$(PYTHON) gen_masm.py --clang $(CLANG) --source asm_x64_win.S --output $@

clean:
	rm -f $(GENERATED)

.PHONY: all clean
