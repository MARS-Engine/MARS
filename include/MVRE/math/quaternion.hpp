#ifndef MVRE_QUATERNION_
#define MVRE_QUATERNION_

#include "vector4.hpp"
#include "vector3.hpp"
#include <cstddef>
#include "math.hpp"

namespace mvre_math {
    template<typename T> requires arithmetic<T> class quaternion {
    protected:
        vector4<T> m_data;
    public:

        quaternion() {
            m_data = { 0, 0, 0, 1 };
        }

        quaternion(vector3<T> _val, float _w) {
            m_data = { _val.x(), _val.y(), _val.z(), _w };
        }

        explicit quaternion(vector4<T> _val) {
            m_data = _val;
        }

        inline T get(size_t i) const { return m_data[i]; }
        inline void set(size_t i, T value) { m_data[i] = value; }

        inline T& operator[](size_t i) { return m_data[i]; }

        inline float length() {
            return sqrt(m_data[3] * m_data[3] + m_data.xyz().length_squared());
        }

        quaternion<T> normalize() {
            T scale = static_cast<T>(1) / length();
            return quaternion(m_data.xyz() * scale, m_data.w() * scale);
        }

        vector4<T> to_axis_angle() {
            if (abs(m_data.w()) > 1.0f)
                m_data = normalize().m_data;

            vector4<T> result = vector4<T>(0, 0, 0, 2.0f * acos(m_data.w()));

            T den = sqrt(1.0f -  (m_data.w() * m_data.w()));

            if (den > 0.0001f)
                result.xyz(m_data.xyz() / den);
            else
                result.xyz(vector3<T>(1.0f, 0.0f, 0.0f));

            return result;
        }

        quaternion<T> operator*(quaternion right) {
            return quaternion(vector4(
                    m_data.w() * right.m_data.x() + m_data.x() * right.m_data.w() + m_data.y() * right.m_data.z() - m_data.z() * right.m_data.y(),
                    m_data.w() * right.m_data.y() + m_data.y() * right.m_data.w() + m_data.z() * right.m_data.x() - m_data.x() * right.m_data.z(),
                    m_data.w() * right.m_data.z() + m_data.z() * right.m_data.w() + m_data.x() * right.m_data.y() - m_data.y() * right.m_data.x(),
                    m_data.w() * right.m_data.w() - m_data.x() * right.m_data.x() - m_data.y() * right.m_data.y() - m_data.z() * right.m_data.z()
            ));
        }

        vector3<T> operator*(vector3<T> right) {
            vector3 Xyz = m_data.xyz();
            vector3 Uv =  vector3<T>::cross(Xyz, right);
            vector3 Uuv = vector3<T>::cross(Xyz, Uv);

            return right + ((Uv * m_data.w()) + Uuv) * 2.0f;
        }

        void operator*=(quaternion right) {
            m_data = vector4(
                m_data.w() * right.m_data.x() + m_data.x() * right.m_data.w() + m_data.y() * right.m_data.z() - m_data.z() * right.m_data.y(),
                m_data.w() * right.m_data.y() + m_data.y() * right.m_data.w() + m_data.z() * right.m_data.x() - m_data.x() * right.m_data.z(),
                m_data.w() * right.m_data.z() + m_data.z() * right.m_data.w() + m_data.x() * right.m_data.y() - m_data.y() * right.m_data.x(),
                m_data.w() * right.m_data.w() - m_data.x() * right.m_data.x() - m_data.y() * right.m_data.y() - m_data.z() * right.m_data.z()
            );
        }

        bool operator==(const quaternion& _right) const {
            return memcmp(&m_data, &_right.m_data, sizeof(vector4<T>)) == 0;
        }

        static quaternion<T> from_axis_angle(vector3<T> _axis, float _angle) {
            T half = _angle * .5f;
            T s = sin(half);
            return quaternion<T>({
                _axis.x() * s,
                _axis.y() * s,
                _axis.z() * s,
                cos(half)
            });
        }
    };
}

#endif