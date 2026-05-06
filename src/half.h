#include <stdint.h>
#include <type_traits>

extern uint16_t float32_to_float16(float value);
extern float float16_to_float32(uint16_t value);

struct half {
    half() : m_value(0) { }

    operator float() const { return float16_to_float32(m_value); }

    explicit half(float value) : m_value(float32_to_float16(value)) { }

    template <typename T,
              std::enable_if_t<
                  std::is_integral_v<T> || std::is_same_v<double, T>, int> = 0>
    explicit half(T x) : half((float) x) {}

    template <typename T,
              std::enable_if_t<
                  std::is_integral_v<T> || std::is_same_v<double, T>, int> = 0>
    operator T() const {
        return T((float) *this);
    };

private:
    uint16_t m_value;
};

static_assert(sizeof(half) == 2, "Invalid data structure size!");
