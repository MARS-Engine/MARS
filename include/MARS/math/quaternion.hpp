#ifndef MARS_m_dataERNION_
#define MARS_m_dataERNION_

#include "vector4.hpp"
#include "vector3.hpp"
#include <cstddef>
#include "math.hpp"

namespace mars_math {
    template<typename T> requires arithmetic<T> class quaternion {
    protected:
        vector4<T> m_data;
    public:

        quaternion() {
            m_data = { 0, 0, 0, 1 };
        }

        quaternion(vector3<T> _val, float _w) {
            m_data = { _val.x, _val.y, _val.z, _w };
        }

        explicit quaternion(vector4<T> _val) {
            m_data = _val;
        }

        inline T get(size_t i) const { return m_data[i]; }
        inline void set(size_t i, T value) { m_data[i] = value; }

        inline T operator[](size_t i) const { return m_data[i]; }

        inline T& operator[](size_t i) { return m_data[i]; }

        [[nodiscard]] inline float length() const {
            return sqrt(m_data[3] * m_data[3] + m_data.xyz().length_squared());
        }

        [[nodiscard]] quaternion<T> normalize() const {
            T scale = static_cast<T>(1) / length();
            return quaternion(m_data.xyz() * scale, m_data.w * scale);
        }

        [[nodiscard]] vector4<T> to_axis_angle() const noexcept {
            auto data = m_data;
            if (abs(data.w) > 1.0f)
                data = normalize().m_data;

            vector4<T> result = vector4<T>(0, 0, 0, 2.0f * acos(data.w));

            T den = sqrt(1.0f -  (data.w * data.w));

            if (den > 0.0001f)
                result.xyz(data.xyz() / den);
            else
                result.xyz(vector3<T>(1.0f, 0.0f, 0.0f));

            return result;
        }

        [[nodiscard]] vector3<T> to_euler() const {
            vector3<T> euler;

            float sinr_cosp = 2 * (m_data.w * m_data.x + m_data.y * m_data.z);
            float consr_cosp = 1 - 2 * (m_data.x * m_data.x + m_data.y * m_data.y);
            euler.x(atan2(sinr_cosp, consr_cosp));

            float sinp = 2 * (m_data.w * m_data.y - m_data.z * m_data.x);
            if (abs(sinp) >= 1.0f)
                euler.y(copysign(M_PI / 2, sinp));
            else
                euler.y(asin(sinp));

            float siny_cosp = 2 * (m_data.w * m_data.z + m_data.x * m_data.y);
            float cosy_cosp = 1 - 2 * (m_data.y * m_data.y + m_data.z * m_data.z);
            euler.z(atan2(siny_cosp, cosy_cosp));

            return euler;
        }

        quaternion<T> operator*(const quaternion& right) {
            return quaternion(vector4(
                    m_data.w * right.m_data.x + m_data.x * right.m_data.w + m_data.y * right.m_data.z - m_data.z * right.m_data.y,
                    m_data.w * right.m_data.y + m_data.y * right.m_data.w + m_data.z * right.m_data.x - m_data.x * right.m_data.z,
                    m_data.w * right.m_data.z + m_data.z * right.m_data.w + m_data.x * right.m_data.y - m_data.y * right.m_data.x,
                    m_data.w * right.m_data.w - m_data.x * right.m_data.x - m_data.y * right.m_data.y - m_data.z * right.m_data.z
            ));
        }

        vector3<T> operator*(const vector3<T>& right) const noexcept {
            vector3 Xyz = m_data.xyz();
            vector3 Uv =  vector3<T>::cross(Xyz, right);
            vector3 Uuv = vector3<T>::cross(Xyz, Uv);

            return right + ((Uv * m_data.w) + Uuv) * 2.0f;
        }

        void operator*=(const quaternion& right) noexcept {
            m_data = vector4(
                m_data.w * right.m_data.x + m_data.x * right.m_data.w + m_data.y * right.m_data.z - m_data.z * right.m_data.y,
                m_data.w * right.m_data.y + m_data.y * right.m_data.w + m_data.z * right.m_data.x - m_data.x * right.m_data.z,
                m_data.w * right.m_data.z + m_data.z * right.m_data.w + m_data.x * right.m_data.y - m_data.y * right.m_data.x,
                m_data.w * right.m_data.w - m_data.x * right.m_data.x - m_data.y * right.m_data.y - m_data.z * right.m_data.z
            );
        }

        bool operator==(const quaternion& _right) const noexcept {
            return m_data == _right.m_data;
        }

        static quaternion<T> from_axis_angle(const vector3<T>& _axis, float _angle) {
            T half = _angle * .5f;
            T s = sin(half);
            return quaternion<T>({
                _axis.x * s,
                _axis.y * s,
                _axis.z * s,
                cos(half)
            });
        }

        static quaternion<T> from_euler(const vector3<T>& _euler) {
            float cy = cos(_euler.z * 0.5);
            float sy = sin(_euler.z * 0.5);
            float cp = cos(_euler.y * 0.5);
            float sp = sin(_euler.y * 0.5);
            float cr = cos(_euler.x * 0.5);
            float sr = sin(_euler.x * 0.5);

            return quaternion<T>({
                sr * cp * cy - cr * sp * sy,
                cr * sp * cy + sr * cp * sy,
                cr * cp * sy - sr * sp * cy,
                cr * cp * cy + sr * sp * sy
            });
        }
    };
}

#endif