from struct_jit import Struct, Type, ByteOrder
import numpy as np
import pytest


def test01_append_inspect():
    s = Struct().append('c', Type.Float32).append('a', Type.UInt64)
    assert s.size() == 16
    assert s.align() == 8
    fc, fa = s["c"], s["a"]
    assert s[0] is fc and s[1] is fa
    assert fc.offset == 0 and fa.offset == 8
    assert fc.type == Type.Float32 and fa.type == Type.UInt64

    with pytest.raises(RuntimeError):
        s["b"]

    with pytest.raises(IndexError):
        s[2]
    r = 'Struct[\n  float32 c; // @0\n  // 4 bytes of padding\n  uint64 a; // @8\n]'
    assert str(s) == r


def test02_pack():
    s = Struct(pack=True).append('c', Type.Float32).append('a', Type.UInt64)
    assert s.size() == 12
    assert s.align() == 1
    fc, fa = s["c"], s["a"]
    assert s[0] == fc and s[1] == fa
    assert fc.offset == 0 and fa.offset == 4
    assert fc.type == Type.Float32 and fa.type == Type.UInt64
    r = 'Struct[\n  float32 c; // @0\n  uint64 a; // @4\n]'
    assert str(s) == r


@pytest.mark.parametrize('pack', [True, False])
@pytest.mark.parametrize('byte_order', [ByteOrder.LittleEndian, ByteOrder.BigEndian])
def test03_roundtrip_dtype(pack, byte_order):
    s = Struct(pack=pack, byte_order=byte_order)
    s.append('c', Type.Float32).append('a', Type.UInt64)
    dt = s.dtype()
    s2 = Struct(dt)
    print(repr(s))
    print(repr(s2))
    assert s == s2
