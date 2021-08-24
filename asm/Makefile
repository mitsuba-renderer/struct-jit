all: asm_x64_sysv.h asm_x64_win.h

asm_x64_sysv.h: asm_x64_sysv.S asm_x64.S extract.py
	clang -Wall -Wextra -march=sandybridge asm_x64_sysv.S -c -o asm_x64_sysv.o
	python3 extract.py asm_x64_sysv.o asm_x64_sysv.h

asm_x64_win.h: asm_x64_win.S asm_x64.S extract.py
	clang -Wall -Wextra -march=sandybridge asm_x64_win.S -c -o asm_x64_win.o
	python3 extract.py asm_x64_win.o asm_x64_win.h
