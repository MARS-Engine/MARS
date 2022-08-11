#ifndef __MATH__HASH__
#define __MATH__HASH__

#include "Vector3.hpp"

namespace std {
    template<> struct hash<Vector3> {
        size_t operator()(Vector3 const& vec3) const {
            return Vector3()(vec3);
        }
    };

    template<> struct hash<Vector2> {
        size_t operator()(Vector2 const& vec2) const {
            return Vector2()(vec2);
        }
    };
}


#endif