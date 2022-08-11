#ifndef __VERTEXHASH__
#define __VERTEXHASH__

#include "Math/Hash.hpp"
#include "Vertex3.hpp"

namespace std {
    template<> struct hash<Vertex3> {
        size_t operator()(Vertex3 const& vertex) const {
            return ((hash<Vector3>()(vertex.position) ^
                     (hash<Vector3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<Vector2>()(vertex.uv) << 1);
        }
    };
}

#endif