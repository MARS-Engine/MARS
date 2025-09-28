#pragma once

#include "annotation.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include <cmath>
#include <mars/container/array.hpp>

namespace mars {

    template <typename T>
    struct[[= mars::matrix(4, 4)]] matrix4 {
        array<vector4<T>, 4> columns = {};

        matrix4() = default;

        matrix4(const T& _value) {
            columns[0, 0] = columns[1, 1] = columns[2, 2] = columns[3, 3] = _value;
        }

        matrix4(const T& _scale_x, const T& _scale_y, const T& _scale_z) {
            columns[0, 0] = _scale_x;
            columns[1, 1] = _scale_y;
            columns[2, 2] = _scale_z;
            columns[3, 3] = 1.0f;
        }

        matrix4(array<vector4<T>, 4> _mat) : columns(_mat) {}

        vector4<T> operator*(const vector4<T>& _value) {
            return _value[0] * columns[0] +
                   _value[1] * columns[1] +
                   _value[2] * columns[2] +
                   _value[3] * columns[3];
        }

        matrix4 operator*(const matrix4& _right) const {
            matrix4 result;
            for (int y = 0; y < 4; ++y) {
                const vector4<T>& right_col = _right.columns[y];
                result.columns[y] =
                    right_col[0] * columns[0] +
                    right_col[1] * columns[1] +
                    right_col[2] * columns[2] +
                    right_col[3] * columns[3];
            }
            return result;
        }

        matrix4& operator/=(const T& _value) {
            columns[0] /= _value;
            columns[1] /= _value;
            columns[2] /= _value;
            columns[3] /= _value;
            return *this;
        }

        template <typename... I>
        auto& operator[](I... _index) { return columns[_index...]; }

        template <typename... I>
        const auto& operator[](I... _index) const { return columns[_index...]; }
    };

    namespace math {
        template <typename T>
        inline matrix4<T> translate(const matrix4<T>& _mat, const vector3<T>& _value) {
            matrix4<T> trans(1.0f);
            trans[3, 0] = _value.x;
            trans[3, 1] = _value.y;
            trans[3, 2] = _value.z;
            return _mat * trans;
        }

        template <typename T>
        matrix4<T> perspective_fov(T _fovy, T _width, T _height, T _z_near, T _z_far) {
            matrix4<T> result(T(0));

            T const rad = _fovy * M_PI / 180.0f;
            T const h = cos(static_cast<T>(0.5) * rad) / sin(static_cast<T>(0.5) * rad);
            T const w = h * _height / _width;

            result[0, 0] = w;
            result[1, 1] = h;
            result[2, 3] = static_cast<T>(1);

            result[2, 2] = _z_far / (_z_far - _z_near);
            result[3, 2] = -(_z_far * _z_near) / (_z_far - _z_near);

            return result;
        }

        template <typename T>
        matrix4<T> look_at(const vector3<T>& _eye, const vector3<T>& _center, const vector3<T>& _up) {
            vector3<T> const f = normalize(_center - _eye);
            vector3<T> const s = normalize(cross(_up, f));
            vector3<T> const u = cross(f, s);

            matrix4<T> result(1.0f);
            result[0, 0] = s.x;
            result[0, 1] = s.y;
            result[0, 2] = s.z;
            result[1, 0] = u.x;
            result[1, 1] = u.y;
            result[1, 2] = u.z;
            result[2, 0] = f.x;
            result[2, 1] = f.y;
            result[2, 2] = f.z;
            result[0, 3] = -dot(s, _eye);
            result[1, 3] = -dot(u, _eye);
            result[2, 3] = -dot(f, _eye);
            return result;
        }
    } // namespace math
} // namespace mars