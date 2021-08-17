all: impl_x64_sysv.h impl_x64_win.h

impl_x64_sysv.h: impl_x64_sysv.S impl_x64.S extract.py
	clang -march=sandybridge impl_x64_sysv.S -c -o impl_x64_sysv.o
	python3 extract.py impl_x64_sysv.o impl_x64_sysv.h

impl_x64_win.h: impl_x64_win.S impl_x64.S extract.py
	clang -march=sandybridge impl_x64_win.S -c -o impl_x64_win.o
	python3 extract.py impl_x64_win.o impl_x64_win.h
