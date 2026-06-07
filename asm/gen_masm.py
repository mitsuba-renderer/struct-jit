#!/usr/bin/env python3
"""Generate a MASM (ml64) snippet library from the GNU-assembler sources.

MSVC ships only MASM, which cannot parse the GNU-syntax `.S` snippet library
(`.intel_syntax`, `.globl`, C-preprocessor directives, RIP-relative sentinels,
numeric local labels, ...). Rather than maintain a hand-translated MASM dialect
in parallel, we let a real assembler (clang) encode the snippets once and emit
the resulting machine code as plain `DB` byte arrays that ml64 assembles
trivially. The sentinel displacements/immediates baked into the bytes are
patched at run time by the JIT, exactly as for the GAS original, so the raw
template bytes are all that is needed.

This is a *one-time* developer step, not part of the CMake build: the generated
`asm_x64_win.asm` is checked in and compiled as-is on Windows. Re-run it (via
`make -C asm`) only after editing the GAS sources. It runs on any host with
clang + python3 (clang cross-assembles to a temporary ELF object; no binutils
required)."""

import argparse
import struct
import subprocess
import tempfile
from itertools import groupby
from pathlib import Path


def parse_elf(path):
    """Extract (.text bytes, {global symbol: offset}) from a little-endian ELF64.

    The symbol table is the single source of truth: every snippet label is a
    defined global symbol, all in the same (`.text`) section, so no source
    scraping or section-name lookup is needed.

    Branches to a *global* `_end` label (e.g. `jne main_suffix_inner_end + extra`)
    are left as PC-relative relocations rather than resolved inline, so the raw
    section bytes lack the patched displacement. We apply those relocations here
    (`S + A - P`, section-relative) exactly as the linker would, which bakes the
    `extra` sentinel into the bytes the JIT later scans for and patches."""
    data = Path(path).read_bytes()
    if data[:6] != b"\x7fELF\x02\x01":
        raise SystemExit(f"{path}: not a little-endian ELF64 object")
    u = lambda fmt, off: struct.unpack_from(fmt, data, off)[0]

    shoff, shentsize, shnum = u("<Q", 0x28), u("<H", 0x3A), u("<H", 0x3C)
    secs = []  # (sh_type, sh_offset, sh_size, sh_link, sh_info)
    for i in range(shnum):
        b = shoff + i * shentsize
        secs.append((u("<I", b + 4), u("<Q", b + 24), u("<Q", b + 32),
                     u("<I", b + 40), u("<I", b + 44)))

    # The sole symbol table (SHT_SYMTAB == 2); its sh_link names its string table.
    sym_off, sym_size, sym_link = next((o, s, l) for t, o, s, l, _ in secs if t == 2)
    str_off = secs[sym_link][1]

    def name(o):
        return data[str_off + o:data.index(b"\0", str_off + o)].decode()

    syms = []                                       # (value, shndx) by index
    offsets, text_idx = {}, None
    for b in range(sym_off, sym_off + sym_size, 24):
        info, shndx, value = data[b + 4], u("<H", b + 6), u("<Q", b + 8)
        syms.append((value, shndx))
        if info >> 4 == 1 and shndx not in (0, 0xFFF1):  # defined STB_GLOBAL
            offsets[name(u("<I", b))] = value
            text_idx = shndx

    toff, tsize = secs[text_idx][1], secs[text_idx][2]
    text = bytearray(data[toff:toff + tsize])

    # Apply relocations against .text (SHT_RELA == 4 with sh_info == .text).
    for typ, off, size, _, info in secs:
        if typ != 4 or info != text_idx:
            continue
        for b in range(off, off + size, 24):
            r_off, r_info = u("<Q", b), u("<Q", b + 8)
            addend = u("<q", b + 16)
            r_type, r_sym = r_info & 0xFFFFFFFF, r_info >> 32
            sym_val, sym_shndx = syms[r_sym]
            if r_type not in (2, 4) or sym_shndx != text_idx:  # PC32 / PLT32
                raise SystemExit(f"{path}: unsupported relocation (type {r_type})")
            struct.pack_into("<I", text, r_off, (sym_val + addend - r_off) & 0xFFFFFFFF)

    return bytes(text), offsets


def emit_masm(text, offsets, source):
    """Render MASM defining each snippet as labelled `DB` byte arrays."""
    names = sorted(offsets, key=offsets.get)             # by ascending offset
    uniq = sorted(set(offsets.values()))
    end = {o: (uniq[i + 1] if i + 1 < len(uniq) else len(text))
           for i, o in enumerate(uniq)}

    lines = [
        f"; Auto-generated from asm/{Path(source).name} by asm/gen_masm.py.",
        "; DO NOT EDIT. Regenerate with `make -C asm` after editing the GAS",
        "; sources (asm_x64.S / asm_x64_win.S).",
        ";",
        "; Raw x86-64 (Windows ABI) machine-code templates for the JIT snippet",
        "; library, emitted as data bytes so ml64 can assemble them without a",
        "; GNU assembler. The sentinel displacements/immediates are patched at",
        "; run time by the JIT, exactly as in the GAS original.",
        "",
        "OPTION CASEMAP:NONE",
        "",
        "_TEXT SEGMENT",
        "",
    ]
    lines += [f"PUBLIC {n}" for n in names] + [""]

    for off, group in groupby(names, key=offsets.get):  # labels sharing an
        lines += [f"{n} LABEL BYTE" for n in group]     # offset coincide
        chunk = text[off:end[off]]
        for j in range(0, len(chunk), 12):
            lines.append("    DB " + ", ".join(f"0{b:02X}h" for b in chunk[j:j + 12]))

    return "\n".join(lines + ["", "_TEXT ENDS", "END", ""])


def main():
    ap = argparse.ArgumentParser(description="Generate the MASM snippet library.")
    ap.add_argument("--clang", default="clang")
    ap.add_argument("--source", default="asm_x64_win.S")
    ap.add_argument("--output", default="asm_x64_win.asm")
    args = ap.parse_args()

    with tempfile.TemporaryDirectory() as tmp:
        obj = Path(tmp) / "snippets.o"
        subprocess.run([args.clang, "--target=x86_64-unknown-linux-gnu",
                        "-c", args.source, "-o", str(obj)], check=True)
        text, offsets = parse_elf(obj)

    Path(args.output).write_text(emit_masm(text, offsets, args.source))
    print(f"wrote {args.output}: {len(offsets)} symbols, {len(text)} bytes")


if __name__ == "__main__":
    main()
