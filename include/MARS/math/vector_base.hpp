#ifndef MARS_VECTOR_BASE_
#define MARS_VECTOR_BASE_

#include "math_concept.hpp"
#include <string>
#include <cstring>
#include <cmath>
#include <xmmintrin.h>

namespace mars_math {

    template<typename T, size_t Length> requires arithmetic<T> class vector_base {
    protected:
        T m_data[Length] = {0};
    public:

        vector_base() = default;

        vector_base(const vector_base<T, Length>& _val) {
            memcpy(m_data, _val.m_data, sizeof(T) * Length);
        }

        explicit vector_base(T* _val) {
            memcpy(m_data, _val, sizeof(T) * Length);
        }

        inline T get(size_t i) const {
            return m_data[i];
        }

        inline void set(size_t i, T value) {
            m_data[i] = value;
        }

        inline T& operator[](size_t i) {
            return m_data[i];
        }

        vector_base<T, Length>& clamp(T _min, T _max) {
            for (auto& val : m_data)
                val = val < _min ? _min : val > _max ? _max : val;
            return *this;
        }

        vector_base<T, Length>& min(T _min) {
            for (auto& val : m_data)
                val = val < _min ? _min : val;
            return *this;
        }

        vector_base<T, Length>& max(T _max) {
            for (auto& val : m_data)
                val = val > _max ? _max : val;
            return *this;
        }


        vector_base<T, Length> operator+(const vector_base<T, Length>& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] + right.get(i);

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length> operator-(const vector_base<T, Length>& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] - right.get(i);

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length> operator-(const float& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] - right;

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length> operator/(const vector_base<T, Length>& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] / right.get(i);

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length> operator*(const vector_base<T, Length>& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] * right.get(i);

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length> operator*(const float& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] * right;

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length> operator/(const float& right) const {
            T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] / right;

            return vector_base<T, Length>(new_data);
        }

        vector_base<T, Length>& operator+=(const vector_base<T, Length>& right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] += right.get(i);
            return *this;
        }

        vector_base<T, Length>& operator-=(const vector_base<T, Length>& right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] -= right.get(i);
            return *this;
        }

        vector_base<T, Length>& operator*=(const vector_base<T, Length>& right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] *= right.get(i);
            return *this;
        }

        vector_base<T, Length>& operator/=(const vector_base<T, Length>& right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] /= right.get(i);
            return *this;
        }

        vector_base<T, Length>& operator*=(const float& right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] *= right;
            return *this;
        }

        bool operator==(const vector_base<T, Length>& _right) const {
            return memcmp(&m_data, &_right.m_data, sizeof(T) * Length) == 0;
        }
    };

    template<> vector_base<float, 4> vector_base<float, 4>::operator+(const vector_base<float, 4UL> &right) const;
    template<> vector_base<float, 4> vector_base<float, 4>::operator-(const vector_base<float, 4UL> &right) const;
    template<> vector_base<float, 4> vector_base<float, 4>::operator*(const vector_base<float, 4UL> &right) const;
    template<> vector_base<float, 4> vector_base<float, 4>::operator*(const float &right) const;
    template<> vector_base<float, 4> vector_base<float, 4>::operator/(const vector_base<float, 4UL> &right) const;
    template<> vector_base<float, 4> vector_base<float, 4>::operator/(const float &right) const;

    template<> vector_base<float, 4>& vector_base<float, 4>::operator+=(const vector_base<float, 4UL> &right);
    template<> vector_base<float, 4>& vector_base<float, 4>::operator-=(const vector_base<float, 4UL> &right);
    template<> vector_base<float, 4>& vector_base<float, 4>::operator*=(const vector_base<float, 4UL> &right);
    template<> vector_base<float, 4>& vector_base<float, 4>::operator*=(const float &right);
    template<> vector_base<float, 4>& vector_base<float, 4>::operator/=(const vector_base<float, 4UL> &right);
}
#endif