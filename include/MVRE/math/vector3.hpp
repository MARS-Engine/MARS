#ifndef __MVRE__VECTOR3__
#define __MVRE__VECTOR3__

#include "vector_base.hpp"

namespace mvre_math {

    template<typename T> requires arithmetic<T> class vector3 : public vector_base<T, 3> {
    public:
        inline T x() { return this->get(0); }
        inline T y() { return this->get(1); }
        inline T z() { return this->get(2); }

        inline void x(T value) { this[0] = value; }
        inline void y(T value) { this[1] = value; }
        inline void z(T value) { this[2] = value; }

        vector3() : vector_base<T, 3>(new T[]{0, 0, 0}) {  }
        vector3(T _x, T _y) : vector_base<T, 3>(new T[]{_x, _y, 0}) { }
        vector3(T _x, T _y, T _z) : vector_base<T, 3>(new T[]{_x, _y, _z}) { }
    };
}

#endif