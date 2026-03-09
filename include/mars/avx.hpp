#pragma once

#include <immintrin.h>

namespace mars::avx {
#ifdef __AVX2__
static constexpr bool avx2 = true;
#else
static constexpr bool avx2 = false;
#endif

#ifdef __AVX512F__
static constexpr bool avx512 = true;
#else
static constexpr bool avx512 = false;
#endif
}; // namespace mars::avx
