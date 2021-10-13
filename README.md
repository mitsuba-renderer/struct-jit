Struct-JIT
==========

This small self-contained library facilitates efficient conversion from one
kind of structured representation to another. The need for this frequently
arises when dealing with visual information including images or 3D meshes. For
example, consider the following data structures describing positions tagged
with color.

```cpp
struct Source { // <-- Big endian! :(
   uint8_t r, g, b; // in sRGB
   half x, y, z;
};

struct Target { // <-- Little endian!
   float x, y, z;
   float r, g, b, a; // in linear space
};

const size_t size = /* huge number */;
Source input[size];
Target output[size];

convert(input, output, size); // <-- Struct-JIT efficiently performs this step
```

In this example, conversion would involve endianness conversion, reordering,
type conversion, dealing with gamma vs linear encoding, and even inserting
extra fields with a default value (e.g. alpha channel).

While hard-coding such a conversion for a specific type of input and output is
easy, things quickly become messy when the format of the input data structure
``Source`` <em>only becomes known at runtime</em>. This is where Struct-JIT
shines: it generates efficient code to performs the desired conversion on the
fly.

Features
--------

Struct-JIT deals with

- <b>Reordering</b>: input and output fields can be at arbitrary offsets.
- <b>Format conversion</b>: the library can load and save various numeric
  representations (signed/unsigned 8-64 bit integers, half/single/double
  precision floats).
- <b>Endianness conversion</b>: supported on both input and output end.
- <b>Gamma correction</b> (sRGB curve): can be applied or removed.
- <b>Defaults</b>: can substitute missing values with specified defaults.
- <b>Dithering</b>: can apply dithering to avoid banding artifacts when
  producing output with low bit depth (e.g. 8 bit).
- <b>Checks</b>: can check that certain entries have specified default values.

The library provides *C++* and *Python* (optional) interfaces to all
functionality.

Just-in-time compilation
------------------------

The above operations can be arranged in countless ways, which makes it hard to
provide an efficient generic implementation of this functionality. For this
reason, the implementation of this class relies on a JIT compiler that
generates fast specialized conversion code for each specific conversion task.
Generated kernels are cached in memory and reused. The JIT targets the two most
widely architectures, specifically ``x86_64`` (Ivy Bridge or newer) and
``aarch64``.

Due to the basic nature of the task, the JIT compiler admits a particularly
simple and fast implementation that merely copies and pastes pre-compiled
snippets to assemble the final conversion function.

Finally, a slow software emulation mode is also available. Its main application
is to validate the JIT implementation in testcases.
