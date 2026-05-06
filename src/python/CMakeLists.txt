nanobind_add_module(
  struct-jit-python
  STABLE_ABI
  NB_STATIC
  LTO

  main.cpp
  ../half.cpp
)

target_link_libraries(struct-jit-python PRIVATE struct-jit)
target_compile_features(struct-jit-python PRIVATE cxx_std_17)

if (MSVC)
  set(SJIT_OUTPUT_DIRECTORY
    LIBRARY_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit
    LIBRARY_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit)
else()
  set(SJIT_OUTPUT_DIRECTORY
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit)
endif()

set_target_properties(struct-jit-python PROPERTIES
  OUTPUT_NAME struct_jit_ext ${SJIT_OUTPUT_DIRECTORY})

if (APPLE)
  set(SJIT_PYTHON_RPATH "@loader_path")
elseif (UNIX)
  set(SJIT_PYTHON_RPATH "$ORIGIN")
endif()

if (SJIT_MASTER_PROJECT)
  install(TARGETS struct-jit-python LIBRARY DESTINATION struct_jit)
  if (SJIT_PYTHON_RPATH)
    set_target_properties(struct-jit-python PROPERTIES
      INSTALL_RPATH "${SJIT_PYTHON_RPATH}")
  endif()
endif()

if (SJIT_MASTER_PROJECT AND NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../struct_jit/__init__.py
    ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit/__init__.py
    COPYONLY
  )
endif()
unset(SJIT_PYTHON_RPATH)
