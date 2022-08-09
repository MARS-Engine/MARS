#ifndef __MESHRENDERER__
#define __MESHRENDERER__

#include <vector>

#include "Multithread/Component.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/VertexType/Vertex3.hpp"

using namespace std;

class MeshRenderer : public Component {
public:
    Pipeline* pipeline;
    Shader* shader;
    Buffer* buffer;

    void Load() override;
    void Render() override;
};

#endif