#pragma once

/**
 * \file python.h
 *
 * Optional helper for embedders that want to reuse Struct-JIT's nanobind
 * bindings *inside their own extension module* rather than importing the
 * standalone \c struct_jit module. Compile \c src/python/main.cpp with
 * \c SJIT_PYTHON_EMBED defined (which omits its \c NB_MODULE entry point) and
 * call \ref python_export() from your module's initialization.
 */

#include <nanobind/nanobind.h>

namespace struct_jit {
    void python_export(nanobind::module_ &m);
}
