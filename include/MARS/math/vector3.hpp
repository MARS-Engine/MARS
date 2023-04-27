#ifndef MARS_VECTOR3_
#define MARS_VECTOR3_

#include "math_concept.hpp"
#include <cmath>
#include <MARS/debug/debug.hpp>

namespace mars_math {

    template<typename T> requires arithmetic<T> class vector3 {
    public:
        T x = 0;
        T y = 0;
        T z = 0;


        vector3() = default;
        explicit vector3(T _xyz) { x = _xyz; y = _xyz; z = _xyz; }
        vector3(T _x, T _y) { x = _x; y = _y; }
        vector3(T _x, T _y, T _z) { x = _x; y = _y; z = _z; }

        static inline vector3<T> right() { return { 1, 0, 0}; }
        static inline vector3<T> forward() { return { 0, 0, 1}; }
        static inline vector3<T> up() { return { 0, 1, 0}; }

        inline float length() {
            return sqrt(x * x + y * y + z * z);
        }

        inline float length_squared() {
            return x * x + y * y + z * z;
        }

        vector3<T>& normalize() {
            this->operator*=(1.0f / length());
            return *this;
        }

        vector3<T>& clamp(T _min, T _max) {
            x = x < _min ? _min : x > _max ? _max : x;
            y = y < _min ? _min : y > _max ? _max : y;
            z = z < _min ? _min : z > _max ? _max : z;
            return *this;
        }

        static vector3<T> normalize(vector3<T> _val) {
            return _val * (1.0f / _val.length());
        }

        static vector3<T> cross(vector3<T> _left, vector3<T> _right) {
            return vector3<T>(
                    _left.y * _right.z - _right.y * _left.z,
                    _left.z * _right.x - _right.z * _left.x,
                    _left.x * _right.y - _right.x * _left.y
                    );
        }


        static float dot(vector3<T> _left, vector3<T> _right) {
            return _left.x * _right.x + _left.y * _right.y + _left.z * _right.z;
        }

        vector3<T> operator+(const vector3<T>& _right) const noexcept {
            return { x + _right.x, y + _right.y, z + _right.z };
        }

        vector3<T> operator*(const vector3<T>& _right) const noexcept {
            return { x * _right.x, y * _right.y, z * _right.z };
        }

        vector3<T> operator/(const vector3<T>& _right) const noexcept {
            return { x / _right.x, y / _right.y, z / _right.z };
        }

        vector3<T> operator-(const vector3<T>& _right) const noexcept {
            return { x - _right.x, y - _right.y, z - _right.z };
        }

        template<typename C> requires dividable<T, C> vector3<T> operator/(const C& _right) const noexcept {
            return { x / _right, y / _right, z / _right };
        }


        template<typename C> requires multipliable<T, C> vector3<T> operator*(const C& _right) const noexcept {
            return {x * _right, y * _right, z * _right };
        }

        template<typename C> requires subtractable<T, C> vector3<T> operator-(const C& _right) const noexcept {
            return {x - _right, y - _right, z - _right };
        }

        template<typename C> requires multipliable<T, C> vector3<T>& operator*=(const C& _right) {
            x *= _right;
            y *= _right;
            z *= _right;
            return *this;
        }

        vector3<T>& operator*=(const vector3<T>& _right) noexcept {
            x *= _right.x;
            y *= _right.y;
            z *= _right.z;
            return *this;
        }

        vector3<T>& operator+=(const vector3<T>& _right) noexcept {
            x += _right.x;
            y += _right.y;
            z += _right.z;
            return *this;
        }

        vector3<T>& operator-=(const vector3<T>& _right) noexcept {
            x -= _right.x;
            y -= _right.y;
            z -= _right.z;
            return *this;
        }

        bool operator==(const vector3<T>& _right) const noexcept {
            return x == _right.x && y == _right.y && z == _right.z;
        }

        bool operator!=(const vector3<T>& _right) const noexcept {
            return !operator==(_right);
        }

        T& operator[](int _index) noexcept {
            switch (_index) {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return x;
            }
        };

        T operator[](int _index) const noexcept {
            switch (_index) {
                case 0:
                    return x;
                case 1:
                    return y;
                case 2:
                    return z;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return x;
            }
        };
    };
}

#endif