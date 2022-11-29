#ifndef __MVRE__MATRIX4__
#define __MVRE__MATRIX4__

#include "matrix_base.hpp"
#include "quaternion.hpp"

namespace mvre_math {

    template<typename T> requires arithmetic<T> class matrix4: public matrix_base<T, 4, 4> {
    public:
        matrix4() : matrix_base<T, 4, 4>() { }

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
            this->get(0) *= _scale.x();
            this->get(1) *= _scale.y();
            this->get(2) *= _scale.z();
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
                mvre_debug::debug::alert("MVRE - Math - Matrix 4 perspective_fov_lh failed because width is negative");
                return matrix4<T>(1);
            }
            else if (_height < 0) {
                mvre_debug::debug::alert("MVRE - Math - Matrix 4 perspective_fov_lh failed because height is negative");
                return matrix4<T>(1);
            }
            else if (_fov < 0) {
                mvre_debug::debug::alert("MVRE - Math - Matrix 4 perspective_fov_lh failed because fov is negative");
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


    };
}

#endif