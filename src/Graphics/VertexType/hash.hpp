#ifndef __VERTEX__HASH__
#define __VERTEX__HASH__

#include "Math/hash.hpp"
#include "vertex3.hpp"

namespace std {
    template<> struct hash<vertex3> {
        size_t operator()(vertex3 const& vertex) const {
            return ((hash<vector3>()(vertex.position) ^
                     (hash<vector3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<vector2>()(vertex.uv) << 1);
        }
    };
}

#endif