#ifndef MARS_MATH_CONCEPT_
#define MARS_MATH_CONCEPT_

#include <concepts>

namespace mars_math {
    template<typename T> concept arithmetic = std::integral<T> or std::floating_point<T>;
}

#endif