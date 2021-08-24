from elftools.elf.elffile import ELFFile
import io
import struct
import sys

sections = ['loadd', 'loadi', 'stored', 'storei']
types    = ['u8', 'i8', 'u16', 'i16', 'u32', 'i32', 'u64', 'i64', 'f16', 'f32', 'f64']

assert len(sys.argv) == 3

fi = open(sys.argv[1], 'rb')
fo = open(sys.argv[2], 'w')

data_out = io.BytesIO()
data_toc = [0]
data_enum = []

elf = ELFFile(fi)

symtab = elf.get_section_by_name('.symtab')
code = elf.get_section_by_name('.text')
data = code.data()

symbols = sorted(symtab.iter_symbols(), key=lambda x: x.entry.st_value)

for symbol in symbols:
    offset, size = symbol.entry.st_value, symbol.entry.st_size
    if symbol.name.endswith('_end') or symbol.name.endswith('_tmp') or \
       symbol.name == '':
        continue
    data_out.write(data[offset:offset+size])
    data_toc.append(data_out.tell())
    data_enum.append(symbol.name)

def listing(array, fmt, count):
    fo.write('    ')
    for i, v in enumerate(array):
        fo.write(fmt % v)
        if i + 1 < len(array):
            if i % count == count-1:
                fo.write(',\n    ')
            else:
                fo.write(', ')

fo.write('enum class impl {\n')
data_enum.append('last')
listing(data_enum, '%s', 4)
fo.write('\n};\n\n')

fo.write('const uint16_t impl_offset[] = {\n')
listing(data_toc, '0x%04x', 6)
fo.write('\n};\n\n')

data_out.seek(0)
data_out = data_out.read()
fo.write('const uint8_t impl_data[] = {\n')
listing(data_out, '0x%02x', 8)
fo.write('\n};\n\n')

fi.close()
fo.close()
