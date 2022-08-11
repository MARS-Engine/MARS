#ifndef __MESH__
#define __MESH__

#include "Graphics/VertexType/Vertex3.hpp"
#include <vector>
#include <string>

using namespace std;

class Mesh {
public:
    vector<Vertex3> vertices;
    vector<uint32_t> indices;

    void Load(const string& meshLocation);
};

#endif