#ifndef __MVRE__MATH__CONCEPT__
#define __MVRE__MATH__CONCEPT__

#include <concepts>

namespace mvre_math {
    template<typename T> concept arithmetic = std::integral<T> or std::floating_point<T>;
}

#endif