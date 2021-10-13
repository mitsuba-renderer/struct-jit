#include <string.h>

#if defined(__aarch64__)
#  include <arm_neon.h>
#else
#  include <immintrin.h>
#endif

#include "half.h"

template <typename T, typename U> inline T memcpy_cast(const U &val) {
    static_assert(sizeof(T) == sizeof(U), "memcpy_cast: sizes did not match!");
    T result;
    memcpy(&result, &val, sizeof(T));
    return result;
}

uint16_t float32_to_float16(float value) {
    #if defined(__ARM_NEON)
        return memcpy_cast<uint16_t>((__fp16) value);
    #else
        return (uint16_t) _mm_cvtsi128_si32(
            _mm_cvtps_ph(_mm_set_ss(value), _MM_FROUND_CUR_DIRECTION));
    #endif
}

float float16_to_float32(uint16_t value) {
    #if defined(__ARM_NEON)
        return (float) memcpy_cast<__fp16>(value);
    #else
        return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128((int32_t) value)));
    #endif
}
