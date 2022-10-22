#ifndef __MVRE__VECTOR4__
#define __MVRE__VECTOR4__

#include "vector_base.hpp"

namespace mvre_math {

    template<typename T> requires arithmetic<T> class vector4 : public vector_base<T, 4> {
    public:
        inline T x() { return this->get(0); }
        inline T y() { return this->get(1); }
        inline T z() { return this->get(2); }
        inline T w() { return this->get(3); }

        inline void x(T value) { this[0] = value; }
        inline void y(T value) { this[1] = value; }
        inline void z(T value) { this[2] = value; }
        inline void w(T value) { this[3] = value; }

        vector4() : vector_base<T, 4>(new T[]{0, 0, 0, 0}) {  }
        vector4(T _x, T _y) : vector_base<T, 4>(new T[]{_x, _y, 0, 0}) { }
        vector4(T _x, T _y, T _z) : vector_base<T, 4>(new T[]{_x, _y, _z, 0}) { }
        vector4(T _x, T _y, T _z, T _w) : vector_base<T, 4>(new T[]{_x, _y, _z, _w}) { }
    };
}

#endif