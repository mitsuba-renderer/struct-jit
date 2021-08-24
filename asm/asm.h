#pragma once
#include <stdint.h>

#if defined(__aarch64__)
#  include "impl_arm64.h"
#elif defined(_WIN64)
#  include "impl_x64_win.h"
#else
#  include "impl_x64_sysv.h"
#endif
