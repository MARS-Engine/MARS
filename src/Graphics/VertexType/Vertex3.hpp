#ifndef __VERTEX3__
#define __VERTEX3__

#include "Graphics/Vulkan/VTypes.hpp"
#include "../../Math/Vector3.hpp"

struct Vertex3 {
public:
    Vector3 position;
    Vector3 normal;
    Vector2 uv;

    VertexInputDescription GetDescription();

    bool operator==(const Vertex3& other) const;
};

#endif