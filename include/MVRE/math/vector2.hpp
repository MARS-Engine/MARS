#ifndef MVRE_VECTOR2_
#define MVRE_VECTOR2_

#include "vector_base.hpp"

namespace mvre_math {

    template<typename T> requires arithmetic<T> class vector2 : public vector_base<T, 2> {
    public:
        inline T x() { return this->get(0); }
        inline T y() { return this->get(1); }

        inline void x(T value) { this[0] = value; }
        inline void y(T value) { this[1] = value; }

        vector2() : vector_base<T, 2>() { this->set(0, 0); this->set(1, 0); }
        vector2(T _x, T _y)  : vector_base<T, 2>() { this->set(0, _x); this->set(1, _y); }
    };
}

#endif