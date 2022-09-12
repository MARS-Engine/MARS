#ifndef __MESH__
#define __MESH__

#include "Graphics/VertexType/vertex3.hpp"
#include <vector>
#include <string>



class mesh {
public:
    std::vector<vertex3> vertices;
    std::vector<uint32_t> indices;

    void load(const std::string& meshLocation);
};

#endif