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

# Working precisions to exercise every conversion at (single and double).
precisions = [sj.Type.Float32, sj.Type.Float64]

def as_input(data):
    if isinstance(data, np.ndarray):
        return data
    return np.frombuffer(data, dtype=np.uint8)

def check_conversion(conv, src_fmt, dst_fmt, data_in,
                     data_out=None, err_thresh=1e-6):
    src_data = struct.pack(src_fmt, *data_in)
    print(binascii.hexlify(src_data).decode('utf8'))
    converted = conv.convert(as_input(src_data))
    print(binascii.hexlify(converted).decode('utf8'))
    dst_data = struct.unpack(dst_fmt, converted)
    print(dst_data)

    ref = data_out if data_out is not None else data_in
    for i in range(len(dst_data)):
        abs_err = float(dst_data[i]) - float(ref[i])
        assert np.abs(abs_err / (ref[i] + err_thresh)) < err_thresh


def test01_append_inspect():
    s = sj.Struct().append('c', sj.Type.Float32).append('a', sj.Type.UInt64)
    assert s.nbytes() == 16
    assert len(s) == 2
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
    assert s.nbytes() == 12
    assert len(s) == 2
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


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('param', supported_types)
@pytest.mark.parametrize('jit', [False, True])
def test04_roundtrip(param, jit, precision):
    s = sj.Struct().append('val', param[1])
    ss = sj.Converter(s, s, jit=jit, working_precision=precision)
    values = list(range(10))
    if sj.type_is_signed(param[1]):
        values += list(range(-10, 0))
    check_conversion(ss, '@' + param[0] * len(values),
                         '@' + param[0] * len(values),
                         values)


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('source', supported_types_int)
@pytest.mark.parametrize('dest', supported_types_float)
@pytest.mark.parametrize('jit', [False, True])
def test05_roundtrip_normalized(source, dest, jit, precision):
    s1 = sj.Struct().append('val', source[1], sj.Flag.Normalized)
    s2 = sj.Struct().append('val', dest[1])

    s12 = sj.Converter(s1, s2, jit=jit, working_precision=precision)
    s21 = sj.Converter(s2, s1, jit=jit, working_precision=precision)
    max_range = sj.type_range(source[1])[1]

    if dest[1] != sj.Type.Float16:
        values_in = list(range(10))
        err_thresh = 1e-6
    else:
        values_in = [int(max_range/10)*i for i in range(10)]
        err_thresh = 1e-3

    if sj.type_is_signed(source[1]):
        values_in += [-k for k in values_in]

    values_out = [i / max_range for i in values_in]

    print("%s -> %s" % (str(source[1]), str(dest[1])))
    check_conversion(s12, '@' + source[0] * len(values_in),
                          '@' + dest[0] * len(values_out),
                          values_in, values_out, err_thresh)

    print("%s -> %s" % (str(dest[1]), str(source[1])))
    check_conversion(s21, '@' + dest[0] * len(values_out),
                          '@' + source[0] * len(values_in),
                          values_out, values_in, err_thresh)


# Field.value holds a logical value in double precision, so integers are
# represented exactly up to 2**53.
@pytest.mark.parametrize('type_, value', [
    (sj.Type.UInt64, 2**53 - 1),
    (sj.Type.Int64, -7),
    (sj.Type.Float16, 1.5),
    (sj.Type.Float32, 1.5),
    (sj.Type.Float64, 1.5),
])
def test06_field_value_scalar_roundtrip(type_, value):
    f = sj.Field()
    f.type = type_
    f.value = value
    if sj.type_is_float(type_):
        assert f.value == pytest.approx(value, rel=1e-3 if type_ == sj.Type.Float16 else 1e-6)
    else:
        assert f.value == value


def test07_struct_append_integer_and_half_defaults():
    s = sj.Struct()
    s.append('half', sj.Type.Float16, sj.Flag.Default, 1.5)
    s.append('u64', sj.Type.UInt64, sj.Flag.Default, 2**53 - 1)
    s.append('i64', sj.Type.Int64, sj.Flag.Default, -7)
    assert s['half'].value == pytest.approx(1.5, rel=1e-3)
    assert s['u64'].value == 2**53 - 1
    assert s['i64'].value == -7


def test08_manual_layout_is_sorted_and_checked():
    late = sj.Field()
    late.name = 'b'
    late.type = sj.Type.UInt8
    late.offset = 8

    early = sj.Field()
    early.name = 'a'
    early.type = sj.Type.UInt8
    early.offset = 0

    s = sj.Struct()
    s.append(late)
    s.append(early)

    assert s.nbytes() == 9
    assert len(s) == 2
    assert s[0].name == 'a'
    assert s[1].name == 'b'

    overlap = sj.Field()
    overlap.name = 'c'
    overlap.type = sj.Type.UInt16
    overlap.offset = 7

    with pytest.raises(RuntimeError):
        s.append(overlap)


@pytest.mark.parametrize('type_', [sj.Type.Float16, sj.Type.Float32, sj.Type.Float64])
def test09_float_ranges_are_signed(type_):
    lower, upper = sj.type_range(type_)
    assert lower < 0
    assert upper > 0


def test10_validate_rejects_illegal_combinations():
    # 'Normalized' requires an integral field
    with pytest.raises(RuntimeError):
        sj.Struct().append('v', sj.Type.Float32, sj.Flag.Normalized)

    # 'Gamma' requires 'Normalized'
    with pytest.raises(RuntimeError):
        sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Gamma)

    # 'Check' and 'Default' are mutually exclusive
    with pytest.raises(RuntimeError):
        sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Check | sj.Flag.Default)

    # Unknown bits are rejected both at append time and after direct mutation.
    with pytest.raises(RuntimeError):
        sj.Struct().append('v', sj.Type.UInt8, 1 << 31)
    s = sj.Struct().append('v', sj.Type.UInt8)
    s['v'].flags = 1 << 31
    with pytest.raises(RuntimeError):
        sj.Converter(s, s)


def test10b_shared_backend_layout_limits():
    # Field offset arithmetic must not wrap.
    f = sj.Field()
    f.name = 'overflow'
    f.type = sj.Type.UInt64
    f.offset = (1 << 64) - 4
    with pytest.raises(RuntimeError):
        sj.Struct().append(f)

    # Record strides are limited to the common AArch64/x64 JIT capability.
    f = sj.Field()
    f.name = 'too_large'
    f.type = sj.Type.UInt64
    f.offset = 4088
    s = sj.Struct()
    s.append(f)
    with pytest.raises(RuntimeError):
        sj.Converter(s, s, jit=False)
    with pytest.raises(RuntimeError):
        sj.Converter(s, s, jit=True)

    # Unaligned multi-byte offsets above 255 cannot be encoded by every backend.
    f = sj.Field()
    f.name = 'unaligned'
    f.type = sj.Type.UInt16
    f.offset = 257
    s = sj.Struct(pack=True)
    s.append(f)
    with pytest.raises(RuntimeError):
        sj.Converter(s, s, jit=False)
    with pytest.raises(RuntimeError):
        sj.Converter(s, s, jit=True)


def test10c_validate_rejects_mutated_duplicate_names():
    s = sj.Struct().append('a', sj.Type.UInt8).append('b', sj.Type.UInt8)
    s['b'].name = 'a'

    with pytest.raises(RuntimeError):
        s.validate()
    with pytest.raises(RuntimeError):
        sj.Converter(s, s, jit=False)


def test11_create_plan_rejects_unmappable_and_duplicate_weights():
    # An output field with no matching input and no default cannot be planned
    s_in = sj.Struct().append('a', sj.Type.UInt8)
    s_out = sj.Struct().append('b', sj.Type.UInt8)
    with pytest.raises(RuntimeError):
        sj.Converter(s_in, s_out)

    # Multiple weight fields are not allowed
    s = (sj.Struct()
         .append('w1', sj.Type.Float32, sj.Flag.Weight)
         .append('w2', sj.Type.Float32, sj.Flag.Weight))
    with pytest.raises(RuntimeError):
        sj.Converter(s, s)


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('src_fmt,src_type', supported_types_float)
@pytest.mark.parametrize('dst_fmt,dst_type', supported_types_int)
def test12_float_to_int_rounds_to_nearest(src_fmt, src_type, dst_type, dst_fmt, precision):
    # Non-normalized float -> int must round to nearest (ties-to-even), and the
    # JIT and fallback backends must agree bit-for-bit. Values are kept small and
    # non-negative to stay clear of the unclamped out-of-range conversion zone.
    s_in = sj.Struct().append('val', src_type)
    s_out = sj.Struct().append('val', dst_type)
    values = [0.0, 0.4, 0.5, 1.5, 2.5, 2.6, 7.49, 7.51, 100.5, 101.5]

    src_data = struct.pack('@' + src_fmt * len(values), *values)
    jit = sj.Converter(s_in, s_out, jit=True, working_precision=precision).convert(as_input(src_data))
    ref = sj.Converter(s_in, s_out, jit=False, working_precision=precision).convert(as_input(src_data))
    assert jit == ref

    # Reference rounds the values as actually stored in the source type.
    stored = struct.unpack('@' + src_fmt * len(values), src_data)
    expect = [int(np.rint(v)) for v in stored]
    got = struct.unpack('@' + dst_fmt * len(values), ref)
    assert list(got) == expect


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('jit', [False, True])
def test13_weight_division(jit, precision):
    # Converting weighted -> unweighted divides every field by the weight.
    s_in = (sj.Struct()
            .append('w', sj.Type.Float32, sj.Flag.Weight)
            .append('r', sj.Type.Float32)
            .append('g', sj.Type.Float32)
            .append('b', sj.Type.Float32))
    s_out = (sj.Struct()
             .append('r', sj.Type.Float32)
             .append('g', sj.Type.Float32)
             .append('b', sj.Type.Float32))
    c = sj.Converter(s_in, s_out, jit=jit, working_precision=precision)

    out = struct.unpack('@fff', c.convert(as_input(struct.pack('@ffff', 2., 4., 6., 8.))))
    assert out == (2., 3., 4.)

    # Zero weight passes values through unchanged (reciprocal forced to 1).
    out = struct.unpack('@fff', c.convert(as_input(struct.pack('@ffff', 0., 5., 6., 7.))))
    assert out == (5., 6., 7.)

    # A NaN weight propagates into every divided field.
    out = struct.unpack('@fff', c.convert(as_input(struct.pack('@ffff', np.nan, 5., 6., 7.))))
    assert all(np.isnan(v) for v in out)


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('dst_fmt,dst_type', supported_types_int)
def test14_weight_division_integer_matches_fallback(dst_fmt, dst_type, precision):
    # Integer payloads exercise the int<->float path and re-quantization; the two
    # backends must agree bit-for-bit. The data is built from the Struct's own
    # dtype so field offsets are naturally aligned.
    s_in = (sj.Struct()
            .append('w', sj.Type.Float32, sj.Flag.Weight)
            .append('v', dst_type))
    s_out = sj.Struct().append('v', dst_type)

    arr = np.zeros(1, dtype=s_in.dtype())
    arr['w'], arr['v'] = 4.0, 100
    data = arr.tobytes()

    jit = sj.Converter(s_in, s_out, jit=True, working_precision=precision).convert(as_input(data))
    ref = sj.Converter(s_in, s_out, jit=False, working_precision=precision).convert(as_input(data))
    assert jit == ref
    assert np.frombuffer(ref, dtype=s_out.dtype())['v'][0] == 25  # 100 / 4


@pytest.mark.parametrize('precision', precisions)
def test15_weight_kept_when_both_sides_weighted(precision):
    # When the target also carries the weight, no division happens.
    s = (sj.Struct()
         .append('w', sj.Type.Float32, sj.Flag.Weight)
         .append('v', sj.Type.Float32))
    for jit in (False, True):
        c = sj.Converter(s, s, jit=jit, working_precision=precision)
        out = struct.unpack('@ff', c.convert(as_input(struct.pack('@ff', 2., 8.))))
        assert out == (2., 8.)


def _srgb_to_linear(s):
    s = np.asarray(s, dtype=np.float64)
    return np.where(s <= 0.04045, s / 12.92, ((s + 0.055) / 1.055) ** 2.4)


def _linear_to_srgb(l):
    l = np.asarray(l, dtype=np.float64)
    return np.where(l <= 0.0031308, l * 12.92, 1.055 * l ** (1 / 2.4) - 0.055)


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('int_type,np_int,float_type,np_float,atol', [
    (sj.Type.UInt8,  np.uint8,  sj.Type.Float32, np.float32, 1e-6),
    (sj.Type.UInt32, np.uint32, sj.Type.Float64, np.float64, 1e-6),
])
def test16_gamma_decode(int_type, np_int, float_type, np_float, atol, precision):
    # sRGB -> linear is JIT-compiled (not skipped) and agrees with the exact
    # reference within the rational-polynomial's approximation error.
    s_in = sj.Struct().append('v', int_type, sj.Flag.Normalized | sj.Flag.Gamma)
    s_out = sj.Struct().append('v', float_type)
    cj = sj.Converter(s_in, s_out, jit=True, working_precision=precision)
    cf = sj.Converter(s_in, s_out, jit=False, working_precision=precision)
    assert cj.kernel() is not None

    maxv = sj.type_range(int_type)[1]
    raw = np.array([0, 1, int(0.2 * maxv), int(0.5 * maxv), maxv], dtype=np_int)
    out_j = np.frombuffer(cj.convert(raw), dtype=np_float)
    out_f = np.frombuffer(cf.convert(raw), dtype=np_float)
    ref = _srgb_to_linear(raw.astype(np.float64) / maxv)
    assert np.allclose(out_j, ref, atol=atol)
    assert np.allclose(out_j, out_f, atol=atol)


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('jit', [False, True])
def test17_gamma_roundtrip_uint8(jit, precision):
    # uint8 sRGB -> float32 linear -> uint8 sRGB recovers every code exactly.
    s_srgb = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma)
    s_lin = sj.Struct().append('v', sj.Type.Float32)
    dec = sj.Converter(s_srgb, s_lin, jit=jit, working_precision=precision)
    enc = sj.Converter(s_lin, s_srgb, jit=jit, working_precision=precision)
    data = struct.pack('256B', *range(256))
    back = np.frombuffer(enc.convert(as_input(dec.convert(as_input(data)))), dtype=np.uint8)
    assert np.array_equal(back, np.arange(256, dtype=np.uint8))


@pytest.mark.parametrize('precision', precisions)
def test18_gamma_and_weight_compose(precision):
    # A gamma channel that is also weight-divided exercises both features at
    # once; the weight reciprocal register must survive the gamma sequence.
    s_in = (sj.Struct()
            .append('w', sj.Type.Float32, sj.Flag.Weight)
            .append('c', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma))
    s_out = sj.Struct().append('c', sj.Type.Float32)

    arr = np.zeros(1, dtype=s_in.dtype())
    arr['w'], arr['c'] = 2.0, 200
    data = arr.tobytes()

    out_j = np.frombuffer(sj.Converter(s_in, s_out, jit=True, working_precision=precision).convert(as_input(data)), dtype=np.float32)
    out_f = np.frombuffer(sj.Converter(s_in, s_out, jit=False, working_precision=precision).convert(as_input(data)), dtype=np.float32)
    ref = _srgb_to_linear(200 / 255.0) / 2.0
    assert np.allclose(out_j, ref, atol=1e-6)
    assert np.allclose(out_j, out_f, atol=1e-6)


# `precision` selects the dither snippet: dither_f4 (single) or dither_f8 (double).
@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('in_type,np_in', [
    (sj.Type.Float32, np.float32),
    (sj.Type.Float16, np.float16),
])
def test19_dithering(in_type, np_in, precision):
    # Dithering a constant gray spreads the quantized output across the two
    # neighbouring codes while preserving the mean; the backends stay identical.
    s_in = sj.Struct().append('v', in_type)
    s_out = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized)
    data = np.full(4096, np_in(100.4 / 255), dtype=np_in).tobytes()

    kw = dict(working_precision=precision, dither=True)
    j = np.frombuffer(sj.Converter(s_in, s_out, jit=True,  **kw).convert(as_input(data)), dtype=np.uint8)
    f = np.frombuffer(sj.Converter(s_in, s_out, jit=False, **kw).convert(as_input(data)), dtype=np.uint8)
    assert np.array_equal(j, f)
    assert set(j.tolist()) == {100, 101}
    assert abs(j.mean() - 100.4) < 0.2

    # Without dithering the constant input collapses to a single code.
    nd = np.frombuffer(sj.Converter(s_in, s_out, dither=False).convert(as_input(data)), dtype=np.uint8)
    assert set(nd.tolist()) == {100}


@pytest.mark.parametrize('jit', [False, True])
def test20_working_precision(jit):
    # 2^24 + 1 is exact in f64 but rounds to 2^24 in f32, so int32 -> float64
    # distinguishes the single- and double-precision working types. Both backends
    # must agree at each precision.
    s_in = sj.Struct().append('v', sj.Type.Int32)
    s_out = sj.Struct().append('v', sj.Type.Float64)
    data = struct.pack('=i', 16777217)

    single = sj.Converter(s_in, s_out, jit=jit, working_precision=sj.Type.Float32)
    double = sj.Converter(s_in, s_out, jit=jit, working_precision=sj.Type.Float64)
    assert struct.unpack('=d', single.convert(as_input(data)))[0] == 16777216.0
    assert struct.unpack('=d', double.convert(as_input(data)))[0] == 16777217.0


def test21_working_precision_must_be_float():
    s = sj.Struct().append('v', sj.Type.Float32)
    with pytest.raises(RuntimeError):
        sj.Converter(s, s, working_precision=sj.Type.Int32)


@pytest.mark.parametrize('precision', precisions)
def test22_dither_ramp_recovers_gradient(precision):
    # A 512x512 image holding a float32 ramp 0..1 along X (constant down each
    # column), quantized to uint8. Averaging down the Y columns should recover
    # the ramp far more accurately with dithering than with the plain (staircase)
    # quantization. This exercises the full 2D dither index (both axes), and the
    # JIT and fallback must agree per pixel.
    N = 512
    s_in = sj.Struct().append('v', sj.Type.Float32)
    s_out = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized)
    x = np.arange(N, dtype=np.float32) / (N - 1)
    img = np.broadcast_to(x, (N, N)).copy()
    ramp = x.astype(np.float64) * 255.0   # ideal continuous output

    def column_avg(jit, dither):
        c = sj.Converter(s_in, s_out, jit=jit, dither=dither, working_precision=precision)
        out = np.frombuffer(c.convert(img, height=N), dtype=np.uint8).reshape(N, N)
        return out, out.astype(np.float64).mean(axis=0)

    out_dj, avg_dith = column_avg(True,  True)
    out_df, avg_dithf = column_avg(False, True)
    out_pj, _        = column_avg(True,  False)
    out_pf, avg_plain = column_avg(False, False)

    # Backends agree bit-for-bit, dithered and not.
    assert np.array_equal(out_dj, out_df)
    assert np.array_equal(out_pj, out_pf)

    rms = lambda e: float(np.sqrt(np.mean(e ** 2)))
    rms_dither = rms(avg_dith - ramp)
    rms_plain  = rms(avg_plain - ramp)

    assert rms_plain > 0.25              # plain quantization: ~0.29 staircase error
    assert rms_dither < 0.05             # dithering recovers the gradient (~0.022)
    assert rms_dither < rms_plain / 5    # ... by a wide margin


# ---------------------------------------------------------------------------
# Saturation, defaults, Check/Assert, endianness and int->int normalization.
# ---------------------------------------------------------------------------

@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('src_fmt,src_type', supported_types_float)
@pytest.mark.parametrize('dst_fmt,dst_type', supported_types_int)
def test23_float_to_int_saturates(src_fmt, src_type, dst_type, dst_fmt, precision):
    # Out-of-range float -> int must clamp to the destination range (not wrap or
    # produce an indefinite value), and the JIT and fallback must agree bit-for-bit.
    s_in = sj.Struct().append('v', src_type)
    s_out = sj.Struct().append('v', dst_type)
    lo, hi = sj.type_range(dst_type)
    # A large magnitude that the source float can actually represent (float16
    # caps at 65504). For the small int targets this still exceeds the range and
    # exercises saturation; for the wide ones it stays in range.
    big = min(sj.type_range(src_type)[1], 1e30)
    values = [0.0, 0.5, -0.5, 1.5, 100.5, -100.5, big, -big]
    data = struct.pack('@' + src_fmt * len(values), *values)
    jit = sj.Converter(s_in, s_out, jit=True,  working_precision=precision).convert(as_input(data))
    ref = sj.Converter(s_in, s_out, jit=False, working_precision=precision).convert(as_input(data))
    assert jit == ref
    # Every output lies within the destination range (no wrap-around).
    got = struct.unpack('@' + dst_fmt * len(values), ref)
    assert all(lo <= g <= hi for g in got)


def test24_round_and_saturation_int8():
    s1 = sj.Struct().append('v', sj.Type.Float32)
    s2 = sj.Struct().append('v', sj.Type.Int8)
    values = [-0.55, -0.45, 0, 0.45, 0.55, 127, 128, -127, -200]
    expect = [-1, 0, 0, 0, 1, 127, 127, -127, -128]
    data = struct.pack('@' + 'f' * len(values), *values)
    for jit in (False, True):
        out = struct.unpack('@' + 'b' * len(values),
                            sj.Converter(s1, s2, jit=jit).convert(as_input(data)))
        assert list(out) == expect


def test25_round_and_saturation_normalized_int8():
    s1 = sj.Struct().append('v', sj.Type.Float32)
    s2 = sj.Struct().append('v', sj.Type.Int8, sj.Flag.Normalized)
    f = 1.0 / 127.0
    values = [-0.55 * f, -0.45 * f, 0, 0.45 * f, 0.55 * f, 1, 2, -1, -2]
    expect = [-1, 0, 0, 0, 1, 127, 127, -127, -128]
    data = struct.pack('@' + 'f' * len(values), *values)
    for jit in (False, True):
        out = struct.unpack('@' + 'b' * len(values),
                            sj.Converter(s1, s2, jit=jit).convert(as_input(data)))
        assert list(out) == expect


@pytest.mark.parametrize('param', supported_types)
def test26_default_substitution(param):
    # A missing output field with a Default flag is filled in. The JIT path must
    # match the fallback and emit the exact default bits.
    fmt, t = param
    s1 = sj.Struct().append('val1', t).append('val3', t)
    s2 = (sj.Struct().append('val1', t)
                     .append('val2', t, sj.Flag.Default, 7)
                     .append('val3', t))
    data = struct.pack('@' + fmt * 2, 3, 5)
    jit = sj.Converter(s1, s2, jit=True).convert(as_input(data))
    ref = sj.Converter(s1, s2, jit=False).convert(as_input(data))
    assert jit == ref
    assert struct.unpack('@' + fmt * 3, jit) == (3, 7, 5)


def test27_default_big_endian():
    # A big-endian default is byte-swapped like any other output field.
    s1 = sj.Struct().append('a', sj.Type.UInt16)
    s2 = (sj.Struct(byte_order=sj.ByteOrder.BigEndian)
            .append('a', sj.Type.UInt16)
            .append('d', sj.Type.UInt32, sj.Flag.Default, 0x11223344))
    data = struct.pack('@H', 7)
    jit = sj.Converter(s1, s2, jit=True).convert(as_input(data))
    ref = sj.Converter(s1, s2, jit=False).convert(as_input(data))
    assert jit == ref
    a, d = struct.unpack('>HxxI', jit)   # a@0 (uint16), 2 pad, d@4 (uint32)
    assert a == 7 and d == 0x11223344


@pytest.mark.parametrize('param', supported_types)
def test28_check_assert(param):
    # A Check-flagged input field must match its expected value or the whole
    # conversion fails. Covers both a mapped field and an input-only field, in
    # the JIT and the fallback.
    fmt, t = param

    # (a) Check field that is also copied to the output.
    s = sj.Struct().append('v', t, sj.Flag.Check, 10)
    so = sj.Struct().append('v', t)
    cj = sj.Converter(s, so, jit=True)
    cf = sj.Converter(s, so, jit=False)
    assert cj.convert(as_input(struct.pack('@' + fmt, 10))) == cf.convert(as_input(struct.pack('@' + fmt, 10)))
    for c in (cj, cf):
        with pytest.raises(RuntimeError):
            c.convert(as_input(struct.pack('@' + fmt, 11)))

    # (b) Check field present only in the input (not mapped to any output).
    s2 = sj.Struct().append('v1', t, sj.Flag.Check, 10).append('v2', t)
    so2 = sj.Struct().append('v2', t)
    for jit in (False, True):
        c = sj.Converter(s2, so2, jit=jit)
        out = c.convert(as_input(struct.pack('@' + fmt * 2, 10, 3)))
        assert struct.unpack('@' + fmt, out)[0] == 3
        with pytest.raises(RuntimeError):
            c.convert(as_input(struct.pack('@' + fmt * 2, 11, 3)))


@pytest.mark.parametrize('param', supported_types)
def test29_endianness_conversion(param):
    # Endianness conversion on both ends must preserve values and stay
    # jit==fallback.
    fmt, t = param
    le, be = sj.ByteOrder.LittleEndian, sj.ByteOrder.BigEndian
    values = list(range(8))
    if sj.type_is_signed(t):
        values += [-v for v in range(1, 5)]

    for src_bo, dst_bo, src_e, dst_e in [(le, be, '<', '>'), (be, le, '>', '<')]:
        s1 = sj.Struct(byte_order=src_bo).append('v', t)
        s2 = sj.Struct(byte_order=dst_bo).append('v', t)
        data = struct.pack(src_e + fmt * len(values), *values)
        jit = sj.Converter(s1, s2, jit=True).convert(as_input(data))
        ref = sj.Converter(s1, s2, jit=False).convert(as_input(data))
        assert jit == ref
        assert list(struct.unpack(dst_e + fmt * len(values), jit)) == values


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('dst_fmt,dst_type', supported_types_int)
def test30_int_to_int_normalized(dst_fmt, dst_type, precision):
    # Normalized int -> normalized int requantizes through the working float.
    # JIT and fallback agree, 0 maps to 0, and full scale maps to (essentially)
    # the destination max.
    s1 = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized)
    s2 = sj.Struct().append('v', dst_type, sj.Flag.Normalized)
    dst_hi = sj.type_range(dst_type)[1]
    vin = list(range(0, 256, 17)) + [255]
    data = struct.pack('@' + 'B' * len(vin), *vin)
    jit = sj.Converter(s1, s2, jit=True,  working_precision=precision).convert(as_input(data))
    ref = sj.Converter(s1, s2, jit=False, working_precision=precision).convert(as_input(data))
    assert jit == ref
    out = struct.unpack('@' + dst_fmt * len(vin), ref)
    assert out[0] == 0
    assert out[-1] >= int(dst_hi * 0.99999)             # full scale -> ~max
    assert all(out[i] <= out[i + 1] for i in range(len(out) - 1))   # monotonic


@pytest.mark.parametrize('precision', precisions)
@pytest.mark.parametrize('src_fmt,src_type', supported_types_int)
@pytest.mark.parametrize('dst_fmt,dst_type', supported_types_int)
def test31_int_to_int_saturates(src_fmt, src_type, dst_fmt, dst_type, precision):
    # Non-normalized integer -> integer conversions route through the working
    # float and SATURATE to the destination range, rather than performing a
    # C-style cast that wraps/truncates on overflow; clamping is the more useful
    # behavior. The JIT and fallback must still agree bit-for-bit, and every
    # result must lie inside the target range.
    lo, hi = sj.type_range(dst_type)
    src_lo, src_hi = sj.type_range(src_type)

    # A spread of in-range, below-range and above-range source values (clamped
    # to what the source type can actually hold). Small magnitudes stay exact in
    # both working precisions; the large ones exercise saturation.
    cand = [0, 1, 5, 100, 127, 128, 255, 256, 32767, 32768, 65535,
            -1, -5, -128, -129, -32768, 100000, -100000, 2**31, -2**31]
    values = sorted({int(max(src_lo, min(src_hi, v))) for v in cand})

    s_in = sj.Struct().append('v', src_type)
    s_out = sj.Struct().append('v', dst_type)
    data = struct.pack('@' + src_fmt * len(values), *values)
    jit = sj.Converter(s_in, s_out, jit=True,  working_precision=precision).convert(as_input(data))
    ref = sj.Converter(s_in, s_out, jit=False, working_precision=precision).convert(as_input(data))
    assert jit == ref

    got = struct.unpack('@' + dst_fmt * len(values), ref)
    # Every output is genuinely within range (no wrap-around), at both precisions.
    assert all(lo <= g <= hi for g in got)
    # Exact saturating semantics: result == clamp(value, dst_lo, dst_hi). This
    # only holds bit-exactly when the source value survives the float round-trip;
    # the candidate magnitudes (<= 2^31) are all exact in float64 but not float32
    # (e.g. the int32 max rounds up), so the strong check is gated on f64.
    if precision == sj.Type.Float64:
        expect = [int(max(lo, min(hi, v))) for v in values]
        assert list(got) == expect


@pytest.mark.parametrize('byte_orders,src_e,dst_e', [
    ((sj.ByteOrder.Native, sj.ByteOrder.Native), '@', '@'),
    ((sj.ByteOrder.BigEndian, sj.ByteOrder.LittleEndian), '>', '<'),
    ((sj.ByteOrder.LittleEndian, sj.ByteOrder.BigEndian), '<', '>'),
])
@pytest.mark.parametrize('dst', supported_types)
@pytest.mark.parametrize('src', supported_types)
def test32_convert_all_pairs(src, dst, byte_orders, src_e, dst_e):
    # Full Cartesian product of every type -> every type, in native, LE->BE and
    # BE->LE byte orders. Values are small whole numbers that are exactly
    # representable in every type (incl. float16), so a correct conversion
    # reproduces them bit-exactly. The JIT and fallback must also agree
    # byte-for-byte.
    src_fmt, src_type = src
    dst_fmt, dst_type = dst
    src_bo, dst_bo = byte_orders

    values = list(range(10))
    if sj.type_is_signed(src_type) and sj.type_is_signed(dst_type):
        values += list(range(-9, 0))

    s1 = sj.Struct(byte_order=src_bo).append('val', src_type)
    s2 = sj.Struct(byte_order=dst_bo).append('val', dst_type)

    data = struct.pack(src_e + src_fmt * len(values), *values)
    jit = sj.Converter(s1, s2, jit=True).convert(as_input(data))
    ref = sj.Converter(s1, s2, jit=False).convert(as_input(data))
    assert jit == ref
    out = struct.unpack(dst_e + dst_fmt * len(values), jit)
    assert list(out) == values


# ---------------------------------------------------------------------------
# Alpha (un)premultiplication.
# ---------------------------------------------------------------------------

@pytest.mark.parametrize('jit', [False, True])
def test33_alpha_premultiply_float(jit):
    # value1 goes premultiplied -> straight (x 1/alpha), value2 goes straight ->
    # premultiplied (x alpha), alpha copied.
    s_in = (sj.Struct()
            .append('value1', sj.Type.Float32, sj.Flag.PremultipliedAlpha)
            .append('value2', sj.Type.Float32)
            .append('alpha',  sj.Type.Float32, sj.Flag.Alpha))
    s_out = (sj.Struct()
             .append('value1', sj.Type.Float32)
             .append('value2', sj.Type.Float32, sj.Flag.PremultipliedAlpha)
             .append('alpha',  sj.Type.Float32, sj.Flag.Alpha))
    c = sj.Converter(s_in, s_out, jit=jit)
    data = struct.pack('@fff', 0.5, 0.8, 0.5)
    out = struct.unpack('@fff', c.convert(as_input(data)))
    assert out == pytest.approx((1.0, 0.4, 0.5))     # 0.5/0.5, 0.8*0.5, copy
    # JIT and fallback agree bit-for-bit (no gamma here).
    other = sj.Converter(s_in, s_out, jit=not jit).convert(as_input(data))
    assert c.convert(as_input(data)) == other


def test34_alpha_multiple_rejected():
    # A premultiplication conversion with more than one alpha channel is
    # ambiguous and must be rejected.
    s_in = (sj.Struct()
            .append('value1', sj.Type.Float32, sj.Flag.PremultipliedAlpha)
            .append('alpha',  sj.Type.Float32, sj.Flag.Alpha)
            .append('alpha2', sj.Type.Float32, sj.Flag.Alpha))
    s_out = (sj.Struct()
             .append('value1', sj.Type.Float32)
             .append('alpha',  sj.Type.Float32, sj.Flag.Alpha)
             .append('alpha2', sj.Type.Float32, sj.Flag.Alpha))
    for jit in (False, True):
        with pytest.raises(RuntimeError):
            sj.Converter(s_in, s_out, jit=jit)


def test35_alpha_multiple_ok_without_conversion():
    # Multiple alpha channels are allowed when no (un)premultiplication is
    # actually requested (here the layouts are equal).
    s = (sj.Struct()
         .append('value1', sj.Type.Float32, sj.Flag.PremultipliedAlpha)
         .append('value2', sj.Type.Float32)
         .append('alpha',  sj.Type.Float32, sj.Flag.Alpha)
         .append('alpha2', sj.Type.Float32, sj.Flag.Alpha))
    data = struct.pack('@ffff', 0.5, 0.8, 0.5, 0.7)
    for jit in (False, True):
        c = sj.Converter(s, s, jit=jit)
        assert struct.unpack('@ffff', c.convert(as_input(data))) == pytest.approx((0.5, 0.8, 0.5, 0.7))


def test36_alpha_uint8_gamma():
    # gamma + normalized + premultiplied uint8.
    s_in = (sj.Struct()
            .append('value1', sj.Type.UInt8, sj.Flag.PremultipliedAlpha | sj.Flag.Normalized | sj.Flag.Gamma)
            .append('value2', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma)
            .append('alpha',  sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Alpha))
    s_out = (sj.Struct()
             .append('value1', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma)
             .append('value2', sj.Type.UInt8, sj.Flag.PremultipliedAlpha | sj.Flag.Normalized | sj.Flag.Gamma)
             .append('alpha',  sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Alpha))
    a = 127 / 255.0
    v0 = _linear_to_srgb(_srgb_to_linear(24 / 255.0) / a)        # unpremultiply
    v1 = _linear_to_srgb(_srgb_to_linear(54 / 255.0) * a)        # premultiply
    ref = (int(np.round(v0 * 255)), int(np.round(v1 * 255)), 127)

    data = struct.pack('@BBB', 24, 54, 127)
    oj = struct.unpack('@BBB', sj.Converter(s_in, s_out, jit=True).convert(as_input(data)))
    of = struct.unpack('@BBB', sj.Converter(s_in, s_out, jit=False).convert(as_input(data)))
    assert oj == of                                              # quantized codes agree
    assert all(abs(g - r) <= 1 for g, r in zip(oj, ref))         # match reference (±1)


# ---------------------------------------------------------------------------
# Blend fields.
# ---------------------------------------------------------------------------

@pytest.mark.parametrize('jit', [False, True])
def test37_blend_float(jit):
    # Linear blend: v = 3*a + 4*b.
    s_in = sj.Struct().append('a', sj.Type.Float32).append('b', sj.Type.Float32)
    s_out = sj.Struct().append('v', sj.Type.Float32)
    s_out['v'].blend = [(3.0, 'a'), (4.0, 'b')]
    c = sj.Converter(s_in, s_out, jit=jit)
    out = struct.unpack('@f', c.convert(as_input(struct.pack('@ff', 1.0, 2.0))))
    assert out[0] == pytest.approx(3.0 * 1.0 + 4.0 * 2.0)


@pytest.mark.parametrize('jit', [False, True])
def test38_blend_normalized(jit):
    # Normalized blend: sources are linearized (here just a [0,1] normalize)
    # before the weighted sum.
    s_in = (sj.Struct()
            .append('a', sj.Type.UInt8, sj.Flag.Normalized)
            .append('b', sj.Type.UInt8, sj.Flag.Normalized))
    s_out = sj.Struct().append('v', sj.Type.Float32)
    s_out['v'].blend = [(3.0, 'a'), (4.0, 'b')]
    c = sj.Converter(s_in, s_out, jit=jit)
    out = struct.unpack('@f', c.convert(as_input(struct.pack('@BB', 255, 127))))
    assert out[0] == pytest.approx(3.0 + 4.0 * (127.0 / 255.0), rel=1e-5)


def test39_blend_gamma():
    # Gamma-decoded sources, summed, then gamma-encoded on output. JIT
    # (polynomial) and fallback (exact) agree on the quantized 8-bit code and
    # both match the reference within one code.
    s_in = (sj.Struct()
            .append('a', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma)
            .append('b', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma))
    s_out = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized | sj.Flag.Gamma)
    s_out['v'].blend = [(1.0, 'a'), (1.0, 'b')]
    data = struct.pack('@BB', 100, 200)
    oj = struct.unpack('@B', sj.Converter(s_in, s_out, jit=True).convert(as_input(data)))[0]
    of = struct.unpack('@B', sj.Converter(s_in, s_out, jit=False).convert(as_input(data)))[0]
    ref = int(np.round(_linear_to_srgb(_srgb_to_linear(100 / 255.0) +
                                       _srgb_to_linear(200 / 255.0)) * 255))
    assert oj == of
    assert abs(oj - ref) <= 1


@pytest.mark.parametrize('jit', [False, True])
def test40_blend_and_weight_compose(jit):
    # A blended output that is also weight-divided: (a + b) / w. Exercises the
    # blend accumulator together with the reserved weight reciprocal register.
    s_in = (sj.Struct()
            .append('w', sj.Type.Float32, sj.Flag.Weight)
            .append('a', sj.Type.Float32)
            .append('b', sj.Type.Float32))
    s_out = sj.Struct().append('v', sj.Type.Float32)
    s_out['v'].blend = [(1.0, 'a'), (1.0, 'b')]
    c = sj.Converter(s_in, s_out, jit=jit)
    out = struct.unpack('@f', c.convert(as_input(struct.pack('@fff', 2.0, 4.0, 6.0))))
    assert out[0] == pytest.approx((4.0 + 6.0) / 2.0)            # (a + b) / w


def test41_packed_unaligned_multibyte_jit_matches_fallback():
    # Packed layouts put the uint16 field at byte offset 1. AArch64 must use an
    # unaligned byte-offset load/store form here rather than truncating the offset
    # through the scaled immediate encoding.
    s = sj.Struct(pack=True).append('a', sj.Type.UInt8).append('b', sj.Type.UInt16)
    data = bytes([0x12, 0x34, 0x56])
    jit = sj.Converter(s, s, jit=True).convert(as_input(data))
    ref = sj.Converter(s, s, jit=False).convert(as_input(data))
    assert jit == ref == data


def test42_struct_rejects_mixed_endian_dtype():
    dt = np.dtype([('a', '>u2'), ('b', '<u2')])
    with pytest.raises(TypeError):
        sj.Struct(dt)


def test42b_struct_rejects_dtype_trailing_padding():
    dt = np.dtype({
        'names': ['a'],
        'formats': ['u1'],
        'offsets': [0],
        'itemsize': 16,
    })
    with pytest.raises(TypeError):
        sj.Struct(dt)


@pytest.mark.parametrize('spec,expected_pack', [
    ({
        'names': ['a', 'b'],
        'formats': ['u1', '<u4'],
        'offsets': [0, 2],
        'itemsize': 6,
    }, True),
    ({
        'names': ['a', 'b'],
        'formats': ['u1', '<u4'],
        'offsets': [0, 2],
        'itemsize': 8,
    }, False),
    ({
        'names': ['a', 'b'],
        'formats': ['u1', 'u1'],
        'offsets': [0, 2],
        'itemsize': 3,
    }, False),
])
def test42c_struct_accepts_representable_dtype_padding(spec, expected_pack):
    dt = np.dtype(spec)
    s = sj.Struct(dt)
    assert s.pack() == expected_pack
    assert s.nbytes() == dt.itemsize
    assert s.dtype() == dt


def test43_convert_accepts_readonly_ndarray():
    s_in = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized)
    s_out = sj.Struct().append('v', sj.Type.Float32)
    data = np.arange(4, dtype=np.uint8)
    data.flags.writeable = False

    out = sj.Converter(s_in, s_out).convert(data)
    values = np.frombuffer(out, dtype=np.float32)
    assert np.allclose(values, data.astype(np.float32) / 255.0)


def test44_converter_cache_is_public_python_api():
    assert hasattr(sj, 'make_converter')
    assert hasattr(sj, 'clear_cache')
    assert sj.native_byte_order() in (sj.ByteOrder.LittleEndian,
                                      sj.ByteOrder.BigEndian)
    assert not hasattr(sj, 'ConverterCache')

    s_in = sj.Struct().append('v', sj.Type.UInt8, sj.Flag.Normalized)
    s_out = sj.Struct().append('v', sj.Type.Float32)
    data = np.array([255], dtype=np.uint8)

    sj.clear_cache()
    cached = sj.make_converter(s_in, s_out)
    out = np.frombuffer(cached.convert(data), dtype=np.float32)
    assert out[0] == 1.0

    fallback = sj.make_converter(s_in, s_out, jit=False)
    assert fallback.kernel() is None
    assert fallback.convert(data) == cached.convert(data)

    del cached, fallback
    sj.clear_cache()
