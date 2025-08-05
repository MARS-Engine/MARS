#pragma once

#include "vector2.hpp"

namespace mars {
    template <typename T>
    struct vector4 {
        T x, y, z, w;

        vector4() = default;
        vector4(const T& _value) : x(_value), y(_value), z(_value), w(_value) {}
        vector4(const T& _x, const T& _y, const T& _z, const T& _w) : x(_x), y(_y), z(_z), w(_w) {}
        vector4(vector2<T> _xy, vector2<T> _zw) : x(_xy.x), y(_xy.y), z(_zw.x), w(_zw.y) {}
        vector4(vector2<T> _xy, const T& _z, const T& _w) : x(_xy.x), y(_xy.y), z(_z), w(_w) {}

        vector2<T>& xy() { return *reinterpret_cast<vector2<T>*>(&x); }
        vector2<T>& zw() { return *reinterpret_cast<vector2<T>*>(&z); }

        const vector2<T>& xy() const { return *reinterpret_cast<vector2<T>*>(&x); }
        const vector2<T>& zw() const { return *reinterpret_cast<vector2<T>*>(&z); }

        vector4 operator*(const T& _value) const {
            return { x * _value, y * _value, z * _value, w * _value };
        }

        vector4 operator+(const vector4& _other) const {
            return { x + _other.x, y + _other.y, z + _other.z, w + _other.w };
        }

        vector4& operator/=(const T& _value) {
            x /= _value;
            y /= _value;
            z /= _value;
            w /= _value;
            return *this;
        }

        T& operator[](size_t _i) {
            return *(&x + _i);
        }

        const T& operator[](size_t _i) const {
            return *(&x + _i);
        }
    };
} // namespace mars