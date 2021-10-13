import struct_jit as sj
import numpy as np
import pytest
import struct
import binascii

# List of supported conversions
supported_types_int = [
    ('b', sj.Type.Int8),
    ('B', sj.Type.UInt8),
    ('h', sj.Type.Int16),
    ('H', sj.Type.UInt16),
    ('i', sj.Type.Int32),
    ('I', sj.Type.UInt32),
    ('q', sj.Type.Int64),
    ('Q', sj.Type.UInt64)
]

supported_types_float = [
    ('e', sj.Type.Float16),
    ('f', sj.Type.Float32),
    ('d', sj.Type.Float64)
]

supported_types = supported_types_int + supported_types_float

def check_conversion(conv, src_fmt, dst_fmt, data_in,
                     data_out=None, err_thresh=1e-6):
    src_data = struct.pack(src_fmt, *data_in)
    print(binascii.hexlify(src_data).decode('utf8'))
    converted = conv.convert(src_data)
    print(binascii.hexlify(converted).decode('utf8'))
    dst_data = struct.unpack(dst_fmt, converted)
    print(dst_data)

    ref = data_out if data_out is not None else data_in
    for i in range(len(dst_data)):
        abs_err = float(dst_data[i]) - float(ref[i])
        assert np.abs(abs_err / (ref[i] + 1e-6)) < err_thresh


def test01_append_inspect():
    s = sj.Struct().append('c', sj.Type.Float32).append('a', sj.Type.UInt64)
    assert s.size() == 16
    assert s.align() == 8
    fc, fa = s["c"], s["a"]
    assert s[0] is fc and s[1] is fa
    assert fc.offset == 0 and fa.offset == 8
    assert fc.type == sj.Type.Float32 and fa.type == sj.Type.UInt64

    with pytest.raises(RuntimeError):
        s["b"]

    with pytest.raises(IndexError):
        s[2]
    r = 'Struct[\n  float32 c; // @0\n  // 4 bytes of padding\n  uint64 a; // @8\n]'
    assert str(s) == r


def test02_pack():
    s = sj.Struct(pack=True).append('c', sj.Type.Float32).append('a', sj.Type.UInt64)
    assert s.size() == 12
    assert s.align() == 1
    fc, fa = s["c"], s["a"]
    assert s[0] == fc and s[1] == fa
    assert fc.offset == 0 and fa.offset == 4
    assert fc.type == sj.Type.Float32 and fa.type == sj.Type.UInt64
    r = 'Struct[\n  float32 c; // @0\n  uint64 a; // @4\n]'
    assert str(s) == r


@pytest.mark.parametrize('pack', [True, False])
@pytest.mark.parametrize('byte_order', [sj.ByteOrder.LittleEndian, sj.ByteOrder.BigEndian])
def test03_roundtrip_dtype(pack, byte_order):
    s = sj.Struct(pack=pack, byte_order=byte_order)
    s.append('c', sj.Type.Float32).append('a', sj.Type.UInt64)
    dt = s.dtype()
    s2 = sj.Struct(dt)
    assert s == s2


@pytest.mark.parametrize('param', supported_types)
@pytest.mark.parametrize('jit', [False, True])
def test04_roundtrip(param, jit):
    s = sj.Struct().append('val', param[1])
    ss = sj.Converter(s, s, jit=jit)
    values = list(range(10))
    if sj.is_signed(param[1]):
        values += list(range(-10, 0))
    check_conversion(ss, '@' + param[0] * len(values),
                         '@' + param[0] * len(values),
                         values)


@pytest.mark.parametrize('source', supported_types_int)
@pytest.mark.parametrize('dest', supported_types_float)
@pytest.mark.parametrize('jit', [False, True])
def test05_roundtrip_normalized(source, dest, jit):
    s1 = sj.Struct().append('val', source[1], sj.Flag.Normalized)
    s2 = sj.Struct().append('val', dest[1])

    s12 = sj.Converter(s1, s2, jit=jit)
    s21 = sj.Converter(s2, s1, jit=jit)

    values_in = list(range(10))
    if sj.is_signed(source[1]):
        values_in += list(range(-10, 0))
    max_range = sj.range(source[1])[1]
    values_out = [i / max_range for i in values_in]

    print("%s -> %s" % (str(source[1]), str(dest[1])))
    check_conversion(s12, '@' + source[0] * len(values_in),
                          '@' + dest[0] * len(values_out),
                          values_in, values_out)

    print("%s -> %s" % (str(dest[1]), str(source[1])))
    check_conversion(s21, '@' + dest[0] * len(values_out),
                          '@' + source[0] * len(values_in),
                          values_out, values_in)
