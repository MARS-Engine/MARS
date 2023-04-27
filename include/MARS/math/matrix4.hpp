#ifndef MARS_MATRIX4_
#define MARS_MATRIX4_

#include "vector4.hpp"
#include "quaternion.hpp"
#include <MARS/debug/debug.hpp>

namespace mars_math {

    template<typename T> requires arithmetic<T> class matrix4 {
    public:
        vector4<T> col0;
        vector4<T> col1;
        vector4<T> col2;
        vector4<T> col3;


        matrix4() = default;

        explicit matrix4(T value) {
            (*this)[0][0] = value;
            (*this)[1][1] = value;
            (*this)[2][2] = value;
            (*this)[3][3] = value;
        }

        matrix4(const vector4<T>& _col0, const vector4<T>& _col1, const vector4<T>& _col2, const vector4<T>& _col3) {
            (*this)[0] = _col0;
            (*this)[1] = _col1;
            (*this)[2] = _col2;
            (*this)[3] = _col3;
        }

        matrix4& translate(vector3<T> _translation) {
            (*this)[3] = (*this)[0]* _translation.x + (*this)[1] * _translation.y + (*this)[2] * _translation.z + (*this)[3];
            return *this;
        }

        matrix4& scale(vector3<T> _scale) {
            (*this)[0] = (*this)[0] * _scale.x;
            (*this)[1] = (*this)[1] * _scale.y;
            (*this)[2] = (*this)[2] * _scale.z;
            return *this;
        }

        static matrix4<T> create_from_axis_angle(const vector4<T>& _axis_angle) {
            vector3<T> norm_axis = _axis_angle.xyz().normalize();

            T _cos = cos(-_axis_angle.w);
            T _sin = sin(-_axis_angle.w);
            T t   = 1.0f - _cos;

            T tXX = t * _axis_angle.x * _axis_angle.x;
            T tXY = t * _axis_angle.x * _axis_angle.y;
            T tXZ = t * _axis_angle.x * _axis_angle.z;
            T tYY = t * _axis_angle.y * _axis_angle.y;
            T tYZ = t * _axis_angle.y * _axis_angle.z;
            T tZZ = t * _axis_angle.z * _axis_angle.z;

            T sinX = _sin * _axis_angle.x;
            T sinY = _sin * _axis_angle.y;
            T sinZ = _sin * _axis_angle.z;

            return {
                    { tXX + _cos, tXY - sinZ, tXZ + sinY, 0 },
                    { tXY + sinZ, tYY + _cos, tYZ - sinX, 0 },
                    { tXZ - sinY, tYZ + sinX, tZZ + _cos, 0 },
                    { 0, 0, 0, 1 }
            };
        }

        static inline matrix4<T> from_quaternion(const quaternion<T>& _quat) noexcept {
            return create_from_axis_angle(_quat.to_axis_angle());
        }

        static matrix4<T> look_at_lh(vector3<T> _eye, vector3<T> _center, vector3<T> _up) {
            vector3<T> f = vector3<T>::normalize(_center - _eye);
            vector3<T> s = vector3<T>::normalize(vector3<T>::cross(_up, f));
            vector3<T> u = vector3<T>::cross(f, s);

            return {
                { s.x, u.x, f.x, 0.0f },
                { s.y, u.y, f.y, 0.0f },
                { s.z, u.z, f.z, 0.0f },
                { -vector3<T>::dot(s, _eye), -vector3<T>::dot(u, _eye), -vector3<T>::dot(f, _eye), 1.0f }
            };
        }

        static matrix4<T> perspective_fov_lh(float _fov, float _width, float _height, float _z_near, float _z_far) {
            if (_width < 0) {
                mars_debug::debug::alert("MARS - Math - Matrix 4 perspective_fov_lh failed because width is negative");
                return matrix4<T>(1);
            }
            else if (_height < 0) {
                mars_debug::debug::alert("MARS - Math - Matrix 4 perspective_fov_lh failed because height is negative");
                return matrix4<T>(1);
            }
            else if (_fov < 0) {
                mars_debug::debug::alert("MARS - Math - Matrix 4 perspective_fov_lh failed because fov is negative");
                return matrix4<T>(1);
            }

            float h = cosf(0.5f * _fov) / sinf(0.5f * _fov);
            float w = h * _height / _width;

            matrix4<T> res;
            res[0][0] = w;
            res[1][1] = h;
            res[2][2] = (_z_far + _z_near) / (_z_far - _z_near);
            res[2][3] = 1.0f;
            res[3][2] = -(2.0f * _z_far * _z_near) / (_z_far - _z_near);
            return res;
        }

        static matrix4<T> ortho_lh(float _left, float _right, float _bottom, float _top, float _z_near, float _z_far) {

            auto Result = matrix4<T>(1);
            Result[0][0] = 2.0f / (_right - _left);
            Result[1][1] = 2.0f / (_top - _bottom);
            Result[2][2] = 1.0f / (_z_far - _z_near);
            Result[3][0] = - (_right + _left) / (_right - _left);
            Result[3][1] = - (_top + _bottom) / (_top - _bottom);
            Result[3][2] = - _z_near / (_z_far - _z_near);
            return Result;
        }

        vector4<T> operator*(const vector4<T>& _right) const noexcept {
            vector4<T> Mov0(_right.x);
            vector4<T> Mov1(_right.y);
            vector4<T> Mul0 = (*this)[0] * Mov0;
            vector4<T> Mul1 = (*this)[1] * Mov1;
            vector4<T> Add0 = Mul0 + Mul1;

            vector4<T> Mov2(_right.z);
            vector4<T> Mov3(_right.w);
            vector4<T> Mul2 = (*this)[2] * Mov2;
            vector4<T> Mul3 = (*this)[3] * Mov3;
            vector4<T> Add1 = Mul2 + Mul3;
            return Add0 + Add1;
        }

        matrix4<T> operator*(const matrix4<T>& _right) const noexcept {
            return {    col0 * _right.col0[0] + col1 * _right.col0[1] + col2 * _right.col0[2] + col3 * _right.col0[3],
                        col0 * _right.col1[0] + col1 * _right.col1[1] + col2 * _right.col1[2] + col3 * _right.col1[3],
                        col0 * _right.col2[0] + col1 * _right.col2[1] + col2 * _right.col2[2] + col3 * _right.col2[3],
                        col0 * _right.col3[0] + col1 * _right.col3[1] + col2 * _right.col3[2] + col3 * _right.col3[3], };
        }

        matrix4<T> operator*=(const matrix4<T>& _right) noexcept {
            *this = *this * _right;
            return *this;
        }

        vector4<T>& operator[](int _index) noexcept {
            switch (_index) {
                case 0:
                    return col0;
                case 1:
                    return col1;
                case 2:
                    return col2;
                case 3:
                    return col3;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return col0;
            }
        };

        vector4<T> operator[](int _index) const noexcept {
            switch (_index) {
                case 0:
                    return col0;
                case 1:
                    return col1;
                case 2:
                    return col2;
                case 3:
                    return col3;
                default:
                    mars_debug::debug::alert("MARS - MATH - Out of bound index");
                    return col0;
            }
        };
    };
}

#endif