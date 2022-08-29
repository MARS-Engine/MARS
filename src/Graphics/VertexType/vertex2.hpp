#ifndef __VERTEX3__
#define __VERTEX3__

#include "Graphics/Vulkan/vtypes.hpp"
#include "../../Math/vector3.hpp"

struct vertex2 {
public:
    vector3 position;
    vector2 uv;

    static VertexInputDescription get_description();

    bool operator==(const vertex2& _other) const;
};

#endif