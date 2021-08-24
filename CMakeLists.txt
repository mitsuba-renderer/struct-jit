cmake_minimum_required(VERSION 3.13...3.18)

project(sjit
  DESCRIPTION "Struct-JIT"
  LANGUAGES CXX
)

option(SJIT_ENABLE_PYTHON "Build Python extension library?" ON)

include(ext/cmake-defaults/CMakeLists.txt)

add_library(
  struct-jit
  SHARED
  include/struct-jit/struct-jit.h
  src/struct-jit.cpp
)

target_compile_features(struct-jit PRIVATE cxx_std_14)
target_include_directories(struct-jit
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if (SJIT_ENABLE_PYTHON)
  message(STATUS "Struct-JIT: building the Python plugin.")
  add_subdirectory(src/python)
else()
  message(STATUS "Struct-JIT: *not* building the Python plugin.")
endif()
