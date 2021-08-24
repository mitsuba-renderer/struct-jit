from importlib import import_module as _import

_import('struct_jit.struct_jit_ext')

def get_cmake_dir():
    from os import path
    file_dir = path.abspath(path.dirname(__file__))
    cmake_path = path.join(file_dir, "share", "cmake", "struct_jit")
    if not path.exists(cmake_path):
        raise ImportError("Cannot find Struct-JIT CMake directory")
    return cmake_path
