#ifndef MARS_VECTOR4_
#define MARS_VECTOR4_

#include "vector_base.hpp"
#include "vector3.hpp"
#include "vector2.hpp"

namespace mars_math {

    template<typename T = float> requires arithmetic<T> class vector4 : public vector_base<T, 4> {
    public:
        inline T x() const { return this->get(0); }
        inline T y() const { return this->get(1); }
        inline T z() const { return this->get(2); }
        inline T w() const { return this->get(3); }

        inline void x(T value) { this->set(0, value); }
        inline void y(T value) { this->set(1, value); }
        inline void z(T value) { this->set(2, value); }
        inline void w(T value) { this->set(3, value); }

        inline vector2<T> xy() { return { x(), y() }; };
        inline vector2<T> zw() { return { z(), w() }; };
        inline vector3<T> xyz() { return { x(), y(), z() }; };
        inline void xyz(vector3<T> _val) { x(_val.x()); y(_val.y()); z(_val.z()); };

        vector4(const vector_base<T, 4>& _val) : vector_base<T, 4>(_val) { }
        vector4() { this->set(0, 0); this->set(1, 0); this->set(2, 0); this->set(3, 0); }
        vector4(T _xyzw) { this->set(0, _xyzw); this->set(1, _xyzw); this->set(2, _xyzw); this->set(3, _xyzw); }
        vector4(T _x, T _y) { this->set(0, _x); this->set(1, _y); this->set(2, 0); this->set(3, 0); }
        vector4(T _x, T _y, T _z) { this->set(0, _x); this->set(1, _y); this->set(2, _z); this->set(3, 0); }
        vector4(T _x, T _y, T _z, T _w) { this->set(0, _x); this->set(1, _y); this->set(2, _z); this->set(3, _w); }
        vector4(vector3<T> _xyz, T _w) { this->set(0, _xyz.x()); this->set(1, _xyz.y()); this->set(2, _xyz.z()); this->set(3, _w); }
    };
}

#endif