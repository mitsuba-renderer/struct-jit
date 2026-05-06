#pragma once

#if !defined(NAMESPACE_BEGIN)
#  define NAMESPACE_BEGIN(name) namespace name {
#endif

#if !defined(NAMESPACE_END)
#  define NAMESPACE_END(name) }
#endif

#include <stdint.h>

NAMESPACE_BEGIN(struct_jit)

enum class ByteOrder : uint32_t;
enum class Type : uint32_t;
enum class Flag : uint32_t;
struct Field;
class Struct;
class Converter;

NAMESPACE_END(struct_jit)
