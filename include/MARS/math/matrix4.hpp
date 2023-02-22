#ifndef MARS_MATRIX4_
#define MARS_MATRIX4_

#include "matrix_base.hpp"
#include "quaternion.hpp"
#include <MARS/debug/debug.hpp>

namespace mars_math {

    template<typename T> requires arithmetic<T> class matrix4: public matrix_base<T, 4, 4> {
    public:
        matrix4() : matrix_base<T, 4, 4>() { }

        using matrix_base<T, 4, 4>::get;
        using matrix_base<T, 4, 4>::set;

        matrix4(const matrix_base<T, 4, 4>& _val) : matrix_base<T,4,4>(_val) { }

        explicit matrix4(T value) : matrix4() {
            this->set(0, 0, value);
            this->set(1, 1, value);
            this->set(2, 2, value);
            this->set(3, 3, value);
        }

        matrix4(vector4<T> _col0, vector4<T> _col1, vector4<T> _col2, vector4<T> _col3) {
            this->set(0, _col0);
            this->set(1, _col1);
            this->set(2, _col2);
            this->set(3, _col3);
        }

        matrix4& translate(vector3<T> _translation) {
            this->set(3, this->get(0) * _translation.x() + this->get(1) * _translation.y() + this->get(2) * _translation.z() + this->get(3));
            return *this;
        }

        matrix4& scale(vector3<T> _scale) {
            set(0, get(0) * _scale.x());
            set(1, get(1) * _scale.y());
            set(2, get(2) * _scale.z());
            return *this;
        }

        static matrix4<T> create_from_axis_angle(vector4<T> _axis_angle) {
            vector3<T> norm_axis = _axis_angle.xyz().normalize();

            T _cos = cos(-_axis_angle.w());
            T _sin = sin(-_axis_angle.w());
            T t   = 1.0f - _cos;

            T tXX = t * _axis_angle.x() * _axis_angle.x();
            T tXY = t * _axis_angle.x() * _axis_angle.y();
            T tXZ = t * _axis_angle.x() * _axis_angle.z();
            T tYY = t * _axis_angle.y() * _axis_angle.y();
            T tYZ = t * _axis_angle.y() * _axis_angle.z();
            T tZZ = t * _axis_angle.z() * _axis_angle.z();

            T sinX = _sin * _axis_angle.x();
            T sinY = _sin * _axis_angle.y();
            T sinZ = _sin * _axis_angle.z();

            return {
                    { tXX + _cos, tXY - sinZ, tXZ + sinY, 0 },
                    { tXY + sinZ, tYY + _cos, tYZ - sinX, 0 },
                    { tXZ - sinY, tYZ + sinX, tZZ + _cos, 0 },
                    { 0, 0, 0, 1 }
            };
        }

        static inline matrix4<T> from_quaternion(quaternion<T> _quat) {
            return create_from_axis_angle(_quat.to_axis_angle());
        }

        static matrix4<T> look_at_lh(vector3<T> _eye, vector3<T> _center, vector3<T> _up) {
            vector3<T> f = vector3<T>::normalize(_center - _eye);
            vector3<T> s = vector3<T>::normalize(vector3<T>::cross(_up, f));
            vector3<T> u = vector3<T>::cross(f, s);

            return {
                { s.x(), u.x(), f.x(), 0.0f },
                { s.y(), u.y(), f.y(), 0.0f },
                { s.z(), u.z(), f.z(), 0.0f },
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
            res.set(0, 0, w);
            res.set(1, 1, h);
            res.set(2, 2, (_z_far + _z_near) / (_z_far - _z_near));
            res.set(2, 3, 1.0f);
            res.set(3, 2, -(2.0f * _z_far * _z_near) / (_z_far - _z_near));
            return res;
        }

        static matrix4<T> ortho_lh(float _left, float _right, float _bottom, float _top, float _z_near, float _z_far) {

            auto Result = matrix4<T>(1);
            Result.set(0,0, 2.0f / (_right - _left));
            Result.set(1,1, 2.0f / (_top - _bottom));
            Result.set(2,2, 1.0f / (_z_far - _z_near));
            Result.set(3,0, - (_right + _left) / (_right - _left));
            Result.set(3,1, - (_top + _bottom) / (_top - _bottom));
            Result.set(3,2, - _z_near / (_z_far - _z_near));
            return Result;
        }

        using matrix_base<T, 4, 4>::operator*;

        vector4<T> operator*(const vector4<T>& _right) const {
            vector4<T> Mov0(_right.x());
            vector4<T> Mov1(_right.y());
            vector4<T> Mul0 = get(0) * Mov0;
            vector4<T> Mul1 = get(1) * Mov1;
            vector4<T> Add0 = Mul0 + Mul1;

            vector4<T> Mov2(_right.z());
            vector4<T> Mov3(_right.w());
            vector4<T> Mul2 = get(2) * Mov2;
            vector4<T> Mul3 = get(3) * Mov3;
            vector4<T> Add1 = Mul2 + Mul3;
            return Add0 + Add1;
        }
    };
}

#endif