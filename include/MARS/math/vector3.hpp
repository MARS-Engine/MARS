#ifndef MARS_VECTOR3_
#define MARS_VECTOR3_

#include "vector_base.hpp"

namespace mars_math {

    template<typename T> requires arithmetic<T> class vector3 : public vector_base<T, 3> {
    public:
        inline T x() const { return this->get(0); }
        inline T y() const { return this->get(1); }
        inline T z() const { return this->get(2); }

        inline void x(T value) { this->set(0, value); }
        inline void y(T value) { this->set(1, value); }
        inline void z(T value) { this->set(2, value); }

        vector3(const vector_base<T, 3>& _val) : vector_base<T, 3>(_val) { }

        vector3() { this->set(0, 0); this->set(1, 0); this->set(2, 0); }
        vector3(T _val) { this->set(0, _val); this->set(1, _val); this->set(2, _val); }
        vector3(T _x, T _y) { this->set(0, _x); this->set(1, _y); this->set(2, 0); }
        vector3(T _x, T _y, T _z) { this->set(0, _x); this->set(1, _y); this->set(2, _z); }

        static inline vector3<T> right() { return { 1, 0, 0}; }
        static inline vector3<T> forward() { return { 0, 0, 1}; }
        static inline vector3<T> up() { return { 0, 1, 0}; }

        inline float length() {
            return sqrt(x() * x() + y() * y() + z() * z());
        }

        inline float length_squared() {
            return x() * x() + y() * y() + z() * z();
        }

        vector3<T>& normalize() {
            vector3<T>::operator*=(1.0f / length());
            return *this;
        }

        static vector3<T> normalize(vector3<T> _val) {
            return _val * (1.0f / _val.length());
        }

        static vector3<T> cross(vector3<T> _left, vector3<T> _right) {
            return vector3<T>(
                    _left.y() * _right.z() - _right.y() * _left.z(),
                    _left.z() * _right.x() - _right.z() * _left.x(),
                    _left.x() * _right.y() - _right.x() * _left.y()
                    );
        }

        static float dot(vector3<T> _left, vector3<T> _right) {
            return _left.x() * _right.x() + _left.y() * _right.y() + _left.z() * _right.z();
        }
    };
}

#endif