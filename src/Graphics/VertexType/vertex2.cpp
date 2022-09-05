#include "vertex2.hpp"

VertexInputDescription vertex2::get_description() {
    VertexInputDescription description;

    VkVertexInputBindingDescription mainBinding = {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(vertex2);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(vertex2, position);

    VkVertexInputAttributeDescription uvAttribute = {};
    uvAttribute.binding = 0;
    uvAttribute.location = 1;
    uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
    uvAttribute.offset = offsetof(vertex2, uv);

    description.attributes.push_back(positionAttribute);
    description.attributes.push_back(uvAttribute);
    return description;
}

bool vertex2::operator==(const vertex2& _other) const {
    return position == _other.position && uv == _other.uv;
}