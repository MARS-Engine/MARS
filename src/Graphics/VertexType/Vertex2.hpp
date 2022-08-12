#ifndef __VERTEX3__
#define __VERTEX3__

#include "Graphics/Vulkan/VTypes.hpp"
#include "../../Math/Vector3.hpp"

struct Vertex2 {
public:
    Vector3 position;
    Vector2 uv;

    static VertexInputDescription GetDescription();

    bool operator==(const Vertex2& other) const;
};

#endif