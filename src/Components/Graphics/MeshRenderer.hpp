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
#include "Loaders/Mesh.hpp"

using namespace std;


class MeshRenderer : public Component {
public:
    Pipeline* pipeline;
    Shader* shader;
    Buffer* verticeBuffer;
    Buffer* indiceBuffer;
    ShaderData* shaderData;
    Texture* texture;
    Mesh* mesh;
    string meshPath;
    bool loaded = false;

    void LoadMesh(const string& meshLocation);
    void Load() override;
    void Update() override;
    void Render() override;
};

#endif