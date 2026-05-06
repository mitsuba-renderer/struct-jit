import struct_jit as sj
import pytest

# The disassembly helper relies on the optional 'capstone' package; skip this
# module when it is not installed.
pytest.importorskip("capstone")


def test_disassemble_contains_ret():
    src = sj.Struct().append('val', sj.Type.UInt8, sj.Flag.Normalized)
    dst = sj.Struct().append('val', sj.Type.Float32)
    conv = sj.Converter(src, dst, jit=True)

    if conv.kernel() is None:
        pytest.skip("no JIT backend compiled on this platform")

    asm = sj.disassemble(conv)
    mnemonics = [line.split()[2] for line in asm.splitlines() if len(line.split()) >= 3]
    # Every kernel returns to its caller ('ret' on both x86-64 and AArch64).
    assert 'ret' in mnemonics
