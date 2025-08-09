#pragma once

#include <immintrin.h>

namespace mars::avx {
    static constexpr bool avx2 = __AVX2__;
    static constexpr bool avx512 = __AVX512F__;
}; // namespace mars::avx