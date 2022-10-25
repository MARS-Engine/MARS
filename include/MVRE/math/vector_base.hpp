#ifndef __MVRE__VECTOR__BASE__
#define __MVRE__VECTOR__BASE__

#include "math_concept.hpp"
#include <string>
#include <cstring>

namespace mvre_math {

    template<typename T, int Length> requires arithmetic<T> class vector_base {
    protected:
        T m_data[Length];
    public:

        vector_base() = default;

        explicit vector_base(T (_val)[Length]) {
            memcpy(m_data, _val, sizeof(T) * Length);
        }

        inline T get(int i) const {
            return m_data[i];
        }

        inline void set(int i, T value) {
            m_data[i] = value;
        }

        inline T& operator[](int i) {
            return m_data[i];
        }

        vector_base<T, Length> operator+(vector_base<T, Length> right) const {
            static T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] + right.get(i);

            return new_data;
        }

        vector_base<T, Length> operator-(vector_base<T, Length> right) const {
            static T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] - right.get(i);

            return new_data;
        }

        vector_base<T, Length> operator/(vector_base<T, Length> right) const {
            static T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] / right.get(i);

            return new_data;
        }

        vector_base<T, Length> operator*(vector_base<T, Length> right) const {
            static T new_data[Length];
            for (auto i = 0; i < Length; i++)
                new_data[i] = m_data[i] * right.get(i);

            return new_data;
        }

        void operator+=(vector_base<T, Length> right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] += right.get(i);
        }

        void operator-=(vector_base<T, Length> right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] -= right.get(i);
        }

        void operator*=(vector_base<T, Length> right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] *= right.get(i);
        }

        void operator/=(vector_base<T, Length> right) {
            for (auto i = 0; i < Length; i++)
                m_data[i] /= right.get(i);
        }
    };
}

#endif