from struct_jit import Struct, Converter, Type, ByteOrder, is_signed
import numpy as np
import pytest
import struct
import binascii

# List of supported conversions
supported_types = [
    ('b', Type.Int8),
    ('B', Type.UInt8),
    ('h', Type.Int16),
    ('H', Type.UInt16),
    ('i', Type.Int32),
    ('I', Type.UInt32),
    ('q', Type.Int64),
    ('Q', Type.UInt64),
    ('e', Type.Float16),
    ('f', Type.Float32),
    ('d', Type.Float64)
]


def check_conversion(conv, src_fmt, dst_fmt, data_in,
                     data_out=None, err_thresh=1e-6):
    src_data = struct.pack(src_fmt, *data_in)
    print(binascii.hexlify(src_data).decode('utf8'))
    converted = conv.convert(src_data)
    print(binascii.hexlify(converted).decode('utf8'))
    dst_data = struct.unpack(dst_fmt, converted)
    ref = data_out if data_out is not None else data_in
    for i in range(len(dst_data)):
        abs_err = float(dst_data[i]) - float(ref[i])
        assert np.abs(abs_err / (ref[i] + 1e-6)) < err_thresh


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
    assert s == s2


@pytest.mark.parametrize('param', supported_types)
def test02_passthrough(param):
    s = Struct().append('val', param[1])
    ss = Converter(s, s)
    values = list(range(10))
    if is_signed(param[1]):
        values += list(range(-10, 0))
    check_conversion(ss, '@' + param[0] * len(values),
                         '@' + param[0] * len(values),
                         values)
