pybind11_add_module(
  struct-jit-python
  THIN_LTO OPT_SIZE

  main.cpp
)

target_link_libraries(struct-jit-python PRIVATE struct-jit)
target_compile_features(struct-jit-python PRIVATE cxx_std_14)

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

if (SJIT_MASTER_PROJECT)
  install(TARGETS struct-jit-python LIBRARY DESTINATION struct-jit)
  set_target_properties(struct-jit-python PROPERTIES INSTALL_RPATH "${SJIT_ORIGIN}")
endif()

if (SJIT_MASTER_PROJECT AND NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../struct_jit/__init__.py
    ${CMAKE_CURRENT_BINARY_DIR}/../../struct_jit/__init__.py
    COPYONLY
  )
endif()


# ----------------------------------------------------------
#   docstring target
# ----------------------------------------------------------

if (UNIX)
  get_filename_component(MKDOC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../include/struct-jit ABSOLUTE)
  get_property(MKDOC_INCLUDE_DIRECTORIES DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  get_property(MKDOC_COMPILE_DEFINITIONS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)

  foreach (value ${MKDOC_INCLUDE_DIRECTORIES})
    list(APPEND MKDOC_CXXFLAGS -I${value})
  endforeach()

  foreach (value ${MKDOC_COMPILE_DEFINITIONS})
    list(APPEND MKDOC_CXXFLAGS -D${value})
  endforeach()

  add_custom_target(docstrings USES_TERMINAL COMMAND
    ${Python_EXECUTABLE} -m pybind11_mkdoc -std=c++17 -stdlib=libc++ -ferror-limit=100000
    ${MKDOC_CXXFLAGS}
    `find ${MKDOC_PATH} -name '*.h' ! -name fwd.h -print`
    -o ${CMAKE_CURRENT_SOURCE_DIR}/docstr.h
  )
endif()
