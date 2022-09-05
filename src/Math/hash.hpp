#ifndef __MATH__HASH__
#define __MATH__HASH__

#include "vector3.hpp"

namespace std {
    template<> struct hash<vector3> {
        size_t operator()(vector3 const& vec3) const {
            return vector3()(vec3);
        }
    };

    template<> struct hash<vector2> {
        size_t operator()(vector2 const& vec2) const {
            return vector2()(vec2);
        }
    };
}


#endif