cmake_minimum_required(VERSION 3.15...3.18)

project(sjit
  VERSION 0.1.0
  DESCRIPTION "Struct-JIT"
  LANGUAGES CXX
)

option(SJIT_ENABLE_PYTHON "Build Python extension library?" ON)

set(SJIT_TARGET_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
if (APPLE AND CMAKE_OSX_ARCHITECTURES)
  # Resolve the effective target arch once so asm selection and default CPU
  # flags agree even when a macOS build targets a non-host architecture.
  set(_sjit_archs "${CMAKE_OSX_ARCHITECTURES}")
  list(REMOVE_DUPLICATES _sjit_archs)
  list(LENGTH _sjit_archs _sjit_arch_count)
  if (NOT _sjit_arch_count EQUAL 1)
    message(FATAL_ERROR "Struct-JIT requires a single target architecture per build directory.")
  endif()
  list(GET _sjit_archs 0 SJIT_TARGET_ARCH)
endif()
unset(_sjit_archs)
unset(_sjit_arch_count)

if (APPLE AND CMAKE_OSX_ARCHITECTURES AND
    NOT "${SJIT_TARGET_ARCH}" STREQUAL "${CMAKE_SYSTEM_PROCESSOR}" AND
    NOT DEFINED CACHE{SJIT_NATIVE_FLAGS})
  if (SJIT_TARGET_ARCH MATCHES "(aarch64|arm64|ARM64)")
    set(SJIT_NATIVE_FLAGS "-mcpu=apple-m1" CACHE STRING
        "Compilation flags used to target the host processor architecture.")
  elseif (SJIT_TARGET_ARCH MATCHES "(x86_64|X86_64|amd64|AMD64)")
    set(SJIT_NATIVE_FLAGS "-march=haswell" CACHE STRING
        "Compilation flags used to target the host processor architecture.")
  endif()
endif()

include(ext/cmake-defaults/CMakeLists.txt)
include(CTest)

set(SJIT_SOURCES
  include/struct-jit/struct-jit.h
  src/struct-jit.cpp
  src/converter.cpp
  src/half.cpp
  src/jit.cpp
  src/dither.cpp
)

# Pick the snippet library for the target arch/ABI. MSVC lacks a GNU assembler,
# so it uses a checked-in MASM translation regenerated via `make -C asm`.
if (SJIT_TARGET_ARCH MATCHES "(aarch64|arm64|ARM64)")
  set(SJIT_ASM_FILE asm/asm_aarch64.S)
elseif (SJIT_TARGET_ARCH MATCHES "(x86_64|X86_64|amd64|AMD64)")
  if (WIN32 AND MSVC)
    set(SJIT_ASM_FILE asm/asm_x64_win.asm)
  elseif (WIN32)
    set(SJIT_ASM_FILE asm/asm_x64_win.S)
  else()
    set(SJIT_ASM_FILE asm/asm_x64_sysv.S)
  endif()
endif()

if (SJIT_ASM_FILE MATCHES "\\.asm$")
  enable_language(ASM_MASM)
else()
  enable_language(ASM)
endif()

list(APPEND SJIT_SOURCES ${SJIT_ASM_FILE})

add_library(struct-jit SHARED ${SJIT_SOURCES})

target_compile_definitions(struct-jit PRIVATE -DSJIT_BUILD=1)
target_compile_features(struct-jit PRIVATE cxx_std_17)
target_include_directories(struct-jit
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/asm
  ${CMAKE_CURRENT_SOURCE_DIR}/ext/nanobind/ext/robin_map/include)

if (MSVC)
  # C4127: conditional expression is constant (robin_map's pre-'if constexpr' code)
  # Scope to C++ only; MASM (ml64) rejects cl.exe flags with warning A4018.
  target_compile_options(struct-jit PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/wd4127>)

  foreach (cfg RELEASE DEBUG RELWITHDEBINFO MINSIZEREL)
    set_target_properties(struct-jit PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/struct_jit
      ARCHIVE_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/struct_jit)
  endforeach()
endif()

if (SJIT_MASTER_PROJECT)
  if (SKBUILD)
    install(TARGETS struct-jit
      RUNTIME DESTINATION struct_jit
      LIBRARY DESTINATION struct_jit
      ARCHIVE DESTINATION struct_jit)
  else()
    install(TARGETS struct-jit
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  endif()
endif()

if (SJIT_ENABLE_PYTHON)
  message(STATUS "Struct-JIT: building the Python plugin.")
  add_subdirectory(src/python)
else()
  message(STATUS "Struct-JIT: *not* building the Python plugin.")
endif()

# ---------------------------------------------------------------------------
# Developer-only target to regenerate src/python/docstr.h from the public
# header. It is NOT part of the default build: docstr.h is checked in and
# compiled as-is, so ordinary builds never depend on the (environment-specific)
# extraction toolchain. Install it with
#   pip install -r docs/requirements.txt
# and regenerate the checked-in file with
#   cmake --build <builddir> --target docstrings   (or: ninja docstrings)
# ---------------------------------------------------------------------------
# Only define this developer convenience target when struct-jit is the top-level
# project; otherwise its name would collide with an embedding project's own.
if (SJIT_MASTER_PROJECT)
  find_package(Python COMPONENTS Interpreter QUIET)
  if (Python_FOUND)
    add_custom_target(docstrings
      COMMAND ${Python_EXECUTABLE} -m pybind11_mkdoc
              -o ${CMAKE_CURRENT_SOURCE_DIR}/src/python/docstr.h
              -w 70 -std=c++17
              -I ${CMAKE_CURRENT_SOURCE_DIR}/include
              -I ${CMAKE_CURRENT_SOURCE_DIR}/include/struct-jit
              ${CMAKE_CURRENT_SOURCE_DIR}/include/struct-jit/struct-jit.h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Regenerating src/python/docstr.h with pybind11_mkdoc"
      VERBATIM USES_TERMINAL)
  endif()
endif()

if (BUILD_TESTING)
  add_executable(struct-jit-cpp-tests tests/test_cpp_api.cpp)
  target_compile_features(struct-jit-cpp-tests PRIVATE cxx_std_17)
  target_link_libraries(struct-jit-cpp-tests PRIVATE struct-jit)
  add_test(NAME struct-jit-cpp-tests COMMAND struct-jit-cpp-tests)
  if (MSVC)
    foreach (cfg RELEASE DEBUG RELWITHDEBINFO MINSIZEREL)
      set_target_properties(struct-jit-cpp-tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/struct_jit)
    endforeach()
  endif()
endif()
