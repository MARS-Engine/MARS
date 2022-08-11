#ifndef __MESHRENDERER__
#define __MESHRENDERER__

#include <vector>

#include "Multithread/Component.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/VertexType/Vertex3.hpp"
#include "Graphics/ShaderData.hpp"

using namespace std;


class MeshRenderer : public Component {
public:
    Pipeline* pipeline;
    Shader* shader;
    Buffer* buffer;
    ShaderData* shaderData;
    Texture* texture;

    void Load() override;
    void Update() override;
    void Render() override;
};

#endif