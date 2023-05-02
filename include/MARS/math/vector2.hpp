#ifndef MARS_VECTOR2_
#define MARS_VECTOR2_

#include "math_concept.hpp"

namespace mars_math {

    template<typename T> requires arithmetic<T> class vector2 {
    public:
        T x = 0;
        T y = 0;

        vector2() = default;
        vector2(T _x, T _y) { x = _x; y = _y; }

        vector2& operator+=(const vector2<T>& _right) noexcept {
            x += _right.x;
            y += _right.y;
            return *this;
        }

        template<typename C> requires addable<T, C> vector2& operator+=(const C& _right) noexcept {
            x += _right;
            y += _right;
            return *this;
        }

        template<typename C> requires multipliable<T, C> vector2& operator*=(const C& _right) noexcept {
            x *= _right;
            y *= _right;
            return *this;
        }

        bool operator==(const vector2<T>& _right) const noexcept {
            return x == _right.x && y == _right.y;
        }

        bool operator!=(const vector2<T>& _right) const noexcept {
            return !operator==(_right);
        }
    };
}

#endif