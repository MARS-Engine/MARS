#ifndef MARS_MATH_CONCEPT_
#define MARS_MATH_CONCEPT_

#include <concepts>

namespace mars_math {
    template<typename T> concept arithmetic = std::integral<T> or std::floating_point<T>;

    template<typename T, typename C> concept multipliable = requires(const T& _left, const C& _right) { _left * _right; };
    template<typename T, typename C> concept dividable = requires(const T& _left, const C& _right) { _left / _right; };
    template<typename T, typename C> concept addable = requires(const T& _left, const C& _right) { _left + _right; };
    template<typename T, typename C> concept subtractable = requires(const T& _left, const C& _right) { _left - _right; };
}

#endif