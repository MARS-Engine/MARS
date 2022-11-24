#ifndef __MVRE__QUATERNION__
#define __MVRE__QUATERNION__

#include "vector4.hpp"
#include "vector3.hpp"
#include <cstddef>
#include "math.hpp"

namespace mvre_math {
    template<typename T> requires arithmetic<T> class quaternion {
    private:
        vector4<T> m_data;
    public:

        quaternion() {
            m_data = { 0, 0, 0, 1 };
        }

        quaternion(vector4<T> _val) {
            m_data = _val;
        }

        inline T get(size_t i) const { return m_data[i]; }
        inline void set(size_t i, T value) { m_data[i] = value; }

        inline T& operator[](size_t i) { return m_data[i]; }

        inline float length() {
            return sqrt(m_data[3] * m_data[3] + m_data.xyz().length_squared());
        }
    };
}

#endif