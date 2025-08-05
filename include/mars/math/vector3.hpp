#pragma once

#include "vector2.hpp"
#include <cmath>
#include <cstddef>

namespace mars {
    template <typename T>
    struct vector3 {
        T x, y, z;

        vector3() = default;
        vector3(const vector2<T>& _xy, T _z) : x(_xy.x), y(_xy.y), z(_z) {}
        vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

        T& operator[](size_t _i) {
            return *(&x + _i);
        }

        const T& operator[](size_t _i) const {
            return *(&x + _i);
        }

        vector3 operator/(const T& _value) const {
            return { x / _value, y / _value, z / _value };
        }

        vector3 operator*(const T& _value) const {
            return { x * _value, y * _value, z * _value };
        }

        vector3 operator+(const vector3& _value) const {
            return { x + _value.x, y + _value.y, z + _value.z };
        }
    };

    template <typename T>
    vector3<T> operator*(const T& _scalar, const vector3<T>& _value) {
        return { _value.x * _scalar, _value.y * _scalar, _value.z * _scalar };
    }

    template <typename T>
    T dot(const vector3<T>& _left, const vector3<T>& _right) {
        return _left.x * _right.x + _left.y * _right.y + _left.z * _right.z;
    }

    template <typename T>
    T length(const vector3<T>& _value) {
        return sqrt(dot(_value, _value));
    }

    template <typename T>
    vector3<T> normalize(const vector3<T>& _value) {
        T len = length(_value);
        if (len > 0)
            return _value / len;
        return {};
    }

    template <typename T>
    T radians(T _degrees) {
        return _degrees * (M_PI / 180.0);
    }
} // namespace mars