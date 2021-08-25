#if !defined(NAMESPACE_BEGIN)
#  define NAMESPACE_BEGIN(name) namespace name {
#endif

#if !defined(NAMESPACE_END)
#  define NAMESPACE_END(name) }
#endif

NAMESPACE_BEGIN(struct_jit)

enum class ByteOrder;
enum class Type;
enum class Flag;
struct Field;

NAMESPACE_END(struct_jit)
