#pragma once

namespace mars {
    template <typename T>
    struct vector2 {
        T x, y;

        vector2 operator*(const T& _value) const {
            return { x * _value, y * _value };
        }

        vector2 operator/(const T& _value) const {
            return { x / _value, y / _value };
        }

        vector2 operator-(const vector2& _value) const {
            return { x - _value.x, y - _value.y };
        }

        vector2 operator+(const vector2& _value) const {
            return { x + _value.x, y + _value.y };
        }

        vector2 operator/(const vector2& _value) const {
            return { x / _value.x, y / _value.y };
        }

        vector2& operator+=(const vector2& _value) {
            x += _value.x;
            y += _value.y;

            return *this;
        }

        vector2& operator/=(const T& _value) {
            x /= _value;
            y /= _value;

            return *this;
        }

        bool operator==(const vector2& _other) {
            return x == _other.x && y == _other.y;
        }

        bool operator!=(const vector2& _other) {
            return !(*this == _other);
        }
    };

    template <typename T>
    vector2<T> operator*(const T& _scalar, const vector2<T>& _value) {
        return { _value.x * _scalar, _value.y * _scalar };
    }
} // namespace mars