from importlib import import_module as _import
from pathlib import Path as _Path

try:
    _import('struct_jit.struct_jit_ext')
except ModuleNotFoundError as e:
    if e.name != 'struct_jit.struct_jit_ext':
        raise
    _build_pkg = _Path(__file__).resolve().parents[1] / 'build' / 'struct_jit'
    if not _build_pkg.exists():
        raise
    __path__.append(str(_build_pkg))
    _import('struct_jit.struct_jit_ext')


def disassemble(converter, *, arch=None):
    '''
    Disassemble the JIT-compiled kernel of a :py:class:`Converter` and return
    the listing as a string.

    Requires the optional `capstone <https://pypi.org/project/capstone>`__
    package. The target architecture is auto-detected from the host machine and
    can be overridden via `arch` (``'arm64'``/``'aarch64'`` or
    ``'x86_64'``/``'amd64'``). Disassembly is a linear sweep, so any constant
    pool emitted after the final ``ret`` will appear as spurious instructions.
    '''
    code = converter.kernel()
    if code is None:
        raise ValueError('Converter has no compiled kernel (jit was disabled '
                         'or no JIT backend exists for this architecture).')

    try:
        import capstone
    except ImportError as e:
        raise ImportError("struct_jit.disassemble() requires the optional "
                          "'capstone' package (e.g. 'pip install capstone').") from e

    import platform
    arch = (arch or platform.machine()).lower()
    if arch in ('arm64', 'aarch64'):
        md = capstone.Cs(capstone.CS_ARCH_ARM64, capstone.CS_MODE_ARM)
    elif arch in ('x86_64', 'amd64', 'x64'):
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    else:
        raise ValueError("struct_jit.disassemble(): unsupported architecture "
                         "%r." % arch)

    lines = []
    for ins in md.disasm(code, 0):
        lines.append('%4x:  %-12s  %s %s' % (ins.address, ins.bytes.hex(),
                                             ins.mnemonic, ins.op_str))
    return '\n'.join(lines)
