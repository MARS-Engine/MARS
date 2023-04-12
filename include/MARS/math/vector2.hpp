#ifndef MARS_VECTOR2_
#define MARS_VECTOR2_

#include "vector_base.hpp"

namespace mars_math {

    template<typename T> requires arithmetic<T> class vector2 : public vector_base<T, 2> {
    public:
        inline T x() const { return this->get(0); }
        inline T y() const { return this->get(1); }

        inline void x(T value) { this->set(0, value); }
        inline void y(T value) { this->set(1, value); }

        vector2(const vector_base<T, 2>& _val) : vector_base<T, 2>(_val) { }

        vector2() : vector_base<T, 2>() { this->set(0, 0); this->set(1, 0); }
        vector2(T _x, T _y)  : vector_base<T, 2>() { this->set(0, _x); this->set(1, _y); }
    };
}

#endif