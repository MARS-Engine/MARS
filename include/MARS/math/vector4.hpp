#ifndef MARS_VECTOR4_
#define MARS_VECTOR4_

#include "math_concept.hpp"
#include "vector3.hpp"
#include "vector2.hpp"
#include <MARS/debug/debug.hpp>

namespace mars_math {

    template<typename T = float> requires arithmetic<T> class vector4 {
    public:
        T x = 0;
        T y = 0;
        T z = 0;
        T w = 0;

        [[nodiscard]] inline vector2<T> xy() const { return {x, y }; };
        [[nodiscard]] inline vector2<T> zw() const { return {z, w }; };
        [[nodiscard]] inline vector3<T> xyz() const { return {x, y, z }; };

        inline void xyz(vector3<T> _val) { x = _val.x; y = _val.y; z = _val.z; };

        vector4() = default;

        explicit vector4(T _xyzw) { x = _xyzw; y = _xyzw; z = _xyzw; w = _xyzw; }
        vector4(T _x, T _y) { x = _x; y = _y; z = 0; w = 0; }
        vector4(T _x, T _y, T _z) { x = _x; y = _y; z = _z; w = 0; }
        vector4(T _x, T _y, T _z, T _w) { x = _x; y = _y; z = _z; w = _w; }
        vector4(vector3<T> _xyz, T _w) { x = _xyz.x; y = _xyz.y; z = _xyz.z; w = _w; }

        vector4<T> operator+(const vector4<T>& _right) const {
            return { x + _right.x, y  + _right.y, z + _right.z, w + _right.w };
        }

        vector4<T> operator*(const vector4<T>& _right) const {
            return { x * _right.x, y  * _right.y, z * _right.z, w * _right.w };
        }

        vector4<T> operator/(const vector4<T>& _right) const {
            return { x / _right.x, y  / _right.y, z / _right.z, w / _right.w };
        }

        template<typename C> requires multipliable<T, C> vector4<T> operator*(const C& _right) const {
            return { x * _right, y * _right, z * _right, w * _right };
        }

        template<typename C> requires addable<T, C> vector4<T> operator+(const C& _right) const {
            return { x + _right, y  + _right, z + _right, w + _right };
        }

        T& operator[](size_t _index) noexcept {
            switch (_index) {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                case 3:
                    return w;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return x;
            }
        }

        T operator[](size_t _index) const noexcept {
            switch (_index) {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                case 3:
                    return w;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return x;
            }
        }

        bool operator==(const vector4<T>& _right) const noexcept {
            return x == _right.x && y == _right.y && z == _right.z && w == _right.w;
        }

        bool operator!=(const vector4<T>& _right) const noexcept {
            return !operator==(_right);
        }
    };
}

#endif