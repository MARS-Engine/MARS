#ifndef __MESH__
#define __MESH__

#include "Graphics/VertexType/vertex3.hpp"
#include <vector>
#include <string>

using namespace std;

class mesh {
public:
    vector<vertex3> vertices;
    vector<uint32_t> indices;

    void load(const string& meshLocation);
};

#endif