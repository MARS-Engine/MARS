#ifndef MVRE_MATH_CONCEPT_
#define MVRE_MATH_CONCEPT_

#include <concepts>

namespace mvre_math {
    template<typename T> concept arithmetic = std::integral<T> or std::floating_point<T>;
}

#endif