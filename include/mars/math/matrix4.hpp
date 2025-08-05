#pragma once

#include "annotation.hpp"
#include "vector3.hpp"
#include "vector4.hpp"
#include <cmath>
#include <mars/container/array.hpp>

namespace mars {

    template <typename T>
    struct[[= mars::matrix(4, 4)]] matrix4 {
        array<vector4<T>, 4> mat = {};

        matrix4() = default;

        matrix4(const T& _value) {
            mat[0, 0] = mat[1, 1] = mat[2, 2] = mat[3, 3] = _value;
        }

        matrix4(const T& _scale_x, const T& _scale_y, const T& _scale_z) {
            mat[0, 0] = _scale_x;
            mat[1, 1] = _scale_y;
            mat[2, 2] = _scale_z;
            mat[3, 3] = 1.0f;
        }

        matrix4(array<vector4<T>, 4> _mat) : mat(_mat) {}

        vector4<T> operator*(const vector4<T>& _value) {
            return {
                mat[0, 0] * _value[0] + mat[0, 1] * _value[1] + mat[0, 2] * _value[2] + mat[0, 3] * _value[3],
                mat[1, 0] * _value[0] + mat[1, 1] * _value[1] + mat[1, 2] * _value[2] + mat[1, 3] * _value[3],
                mat[2, 0] * _value[0] + mat[2, 1] * _value[1] + mat[2, 2] * _value[2] + mat[2, 3] * _value[3],
                mat[3, 0] * _value[0] + mat[3, 1] * _value[1] + mat[3, 2] * _value[2] + mat[3, 3] * _value[3]
            };
        }

        matrix4 operator*(const matrix4& _right) const {
            matrix4 result;
            for (int x = 0; x < 4; ++x) {
                for (int y = 0; y < 4; ++y) {
                    result[x, y] =
                        mat[x, 0] * _right[0, y] +
                        mat[x, 1] * _right[1, y] +
                        mat[x, 2] * _right[2, y] +
                        mat[x, 3] * _right[3, y];
                }
            }
            return result;
        }

        matrix4& operator/=(const T& _value) {
            mat[0] /= _value;
            mat[1] /= _value;
            mat[2] /= _value;
            mat[3] /= _value;
            return *this;
        }

        template <typename... I>
        auto& operator[](I... _index) { return mat[_index...]; }

        template <typename... I>
        const auto& operator[](I... _index) const { return mat[_index...]; }
    };

    namespace math {
        template <typename T>
        inline matrix4<T> translate(const matrix4<T>& _mat, const vector3<T>& _value) {
            matrix4<T> result = _mat;
            result[3, 0] += _value.x;
            result[3, 1] += _value.y;
            result[3, 2] += _value.z;
            return result;
        }

        template <typename T>
        inline matrix4<T> scale(const matrix4<T>& _mat, const vector3<T>& _value) {
            return _mat * matrix4<T>(_value.x, _value.y, _value.z);
        }

        template <typename T>
        matrix4<T> ortho(T _left, T _right, T _bottom, T _top, T _near_z, T _far_z) {
            matrix4<T> result(1);
            result[0, 0] = 2.0f / (_right - _left);
            result[1, 1] = 2.0f / (_top - _bottom);
            result[2, 2] = -2.0f / (_far_z - _near_z);

            result[3, 0] = -(_right + _left) / (_right - _left);
            result[3, 1] = -(_top + _bottom) / (_top - _bottom);
            result[3, 2] = -(_far_z + _near_z) / (_far_z - _near_z);
            return result;
        }

        template <typename T>
        matrix4<T> rotate(const matrix4<T>& _mat, float _angle, const vector3<T>& _value) {
            T c = cos(_angle);
            T s = sin(_angle);
            vector3<T> axis = normalize(_value);
            vector3<T> temp = (T(1) - c) * axis;

            matrix4<T> rotation(1);
            rotation[0, 0] = c + temp.x * axis.x;
            rotation[0, 1] = temp.x * axis.y + s * axis.z;
            rotation[0, 2] = temp.x * axis.z - s * axis.y;
            rotation[1, 0] = temp.y * axis.x - s * axis.z;
            rotation[1, 1] = c + temp.y * axis.y;
            rotation[1, 2] = temp.y * axis.z + s * axis.x;
            rotation[2, 0] = temp.z * axis.x + s * axis.y;
            rotation[2, 1] = temp.z * axis.y - s * axis.x;
            rotation[2, 2] = c + temp.z * axis.z;

            matrix4<T> result;
            result[0] = _mat[0] * rotation[0, 0] + _mat[1] * rotation[0, 1] + _mat[2] * rotation[0, 2];
            result[1] = _mat[0] * rotation[1, 0] + _mat[1] * rotation[1, 1] + _mat[2] * rotation[1, 2];
            result[2] = _mat[0] * rotation[2, 0] + _mat[1] * rotation[2, 1] + _mat[2] * rotation[2, 2];
            result[3] = _mat[3];
            return result;
        }

        template <typename T>
        matrix4<T> transpose(const matrix4<T>& _mat) {
            matrix4<T> result;
            result[0, 0] = _mat[0, 0];
            result[0, 1] = _mat[1, 0];
            result[0, 2] = _mat[2, 0];
            result[0, 3] = _mat[3, 0];
            result[1, 0] = _mat[0, 1];
            result[1, 1] = _mat[1, 1];
            result[1, 2] = _mat[2, 1];
            result[1, 3] = _mat[3, 1];
            result[2, 0] = _mat[0, 2];
            result[2, 1] = _mat[1, 2];
            result[2, 2] = _mat[2, 2];
            result[2, 3] = _mat[3, 2];
            result[3, 0] = _mat[0, 3];
            result[3, 1] = _mat[1, 3];
            result[3, 2] = _mat[2, 3];
            result[3, 3] = _mat[3, 3];
            return result;
        }

        template <typename T>
        matrix4<T> inverse_transpose(const matrix4<T>& _mat) {
            matrix4<T> result;
            T SubFactor00 = _mat[2, 2] * _mat[3, 3] - _mat[3, 2] * _mat[2, 3];
            T SubFactor01 = _mat[2, 1] * _mat[3, 3] - _mat[3, 1] * _mat[2, 3];
            T SubFactor02 = _mat[2, 1] * _mat[3, 2] - _mat[3, 1] * _mat[2, 2];
            T SubFactor03 = _mat[2, 0] * _mat[3, 3] - _mat[3, 0] * _mat[2, 3];
            T SubFactor04 = _mat[2, 0] * _mat[3, 2] - _mat[3, 0] * _mat[2, 2];
            T SubFactor05 = _mat[2, 0] * _mat[3, 1] - _mat[3, 0] * _mat[2, 1];
            T SubFactor06 = _mat[1, 2] * _mat[3, 3] - _mat[3, 2] * _mat[1, 3];
            T SubFactor07 = _mat[1, 1] * _mat[3, 3] - _mat[3, 1] * _mat[1, 3];
            T SubFactor08 = _mat[1, 1] * _mat[3, 2] - _mat[3, 1] * _mat[1, 2];
            T SubFactor09 = _mat[1, 0] * _mat[3, 3] - _mat[3, 0] * _mat[1, 3];
            T SubFactor10 = _mat[1, 0] * _mat[3, 2] - _mat[3, 0] * _mat[1, 2];
            T SubFactor11 = _mat[1, 0] * _mat[3, 1] - _mat[3, 0] * _mat[1, 1];
            T SubFactor12 = _mat[1, 2] * _mat[2, 3] - _mat[2, 2] * _mat[1, 3];
            T SubFactor13 = _mat[1, 1] * _mat[2, 3] - _mat[2, 1] * _mat[1, 3];
            T SubFactor14 = _mat[1, 1] * _mat[2, 2] - _mat[2, 1] * _mat[1, 2];
            T SubFactor15 = _mat[1, 0] * _mat[2, 3] - _mat[2, 0] * _mat[1, 3];
            T SubFactor16 = _mat[1, 0] * _mat[2, 2] - _mat[2, 0] * _mat[1, 2];
            T SubFactor17 = _mat[1, 0] * _mat[2, 1] - _mat[2, 0] * _mat[1, 1];

            matrix4<T> Inverse;
            Inverse[0, 0] = +(_mat[1, 1] * SubFactor00 - _mat[1, 2] * SubFactor01 + _mat[1, 3] * SubFactor02);
            Inverse[0, 1] = -(_mat[1, 0] * SubFactor00 - _mat[1, 2] * SubFactor03 + _mat[1, 3] * SubFactor04);
            Inverse[0, 2] = +(_mat[1, 0] * SubFactor01 - _mat[1, 1] * SubFactor03 + _mat[1, 3] * SubFactor05);
            Inverse[0, 3] = -(_mat[1, 0] * SubFactor02 - _mat[1, 1] * SubFactor04 + _mat[1, 2] * SubFactor05);

            Inverse[1, 0] = -(_mat[0, 1] * SubFactor00 - _mat[0, 2] * SubFactor01 + _mat[0, 3] * SubFactor02);
            Inverse[1, 1] = +(_mat[0, 0] * SubFactor00 - _mat[0, 2] * SubFactor03 + _mat[0, 3] * SubFactor04);
            Inverse[1, 2] = -(_mat[0, 0] * SubFactor01 - _mat[0, 1] * SubFactor03 + _mat[0, 3] * SubFactor05);
            Inverse[1, 3] = +(_mat[0, 0] * SubFactor02 - _mat[0, 1] * SubFactor04 + _mat[0, 2] * SubFactor05);

            Inverse[2, 0] = +(_mat[0, 1] * SubFactor06 - _mat[0, 2] * SubFactor07 + _mat[0, 3] * SubFactor08);
            Inverse[2, 1] = -(_mat[0, 0] * SubFactor06 - _mat[0, 2] * SubFactor09 + _mat[0, 3] * SubFactor10);
            Inverse[2, 2] = +(_mat[0, 0] * SubFactor07 - _mat[0, 1] * SubFactor09 + _mat[0, 3] * SubFactor11);
            Inverse[2, 3] = -(_mat[0, 0] * SubFactor08 - _mat[0, 1] * SubFactor10 + _mat[0, 2] * SubFactor11);

            Inverse[3, 0] = -(_mat[0, 1] * SubFactor12 - _mat[0, 2] * SubFactor13 + _mat[0, 3] * SubFactor14);
            Inverse[3, 1] = +(_mat[0, 0] * SubFactor12 - _mat[0, 2] * SubFactor15 + _mat[0, 3] * SubFactor16);
            Inverse[3, 2] = -(_mat[0, 0] * SubFactor13 - _mat[0, 1] * SubFactor15 + _mat[0, 3] * SubFactor17);
            Inverse[3, 3] = +(_mat[0, 0] * SubFactor14 - _mat[0, 1] * SubFactor16 + _mat[0, 2] * SubFactor17);

            T Determinant = +_mat[0, 0] * Inverse[0, 0] + _mat[0, 1] * Inverse[0, 1] + _mat[0, 2] * Inverse[0, 2] + _mat[0, 3] * Inverse[0, 3];

            Inverse /= Determinant;

            return Inverse;
        }
    } // namespace math
} // namespace mars