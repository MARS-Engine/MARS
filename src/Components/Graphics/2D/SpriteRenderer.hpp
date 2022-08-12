#ifndef __SPRITERENDERER__
#define __SPRITERENDERER__

#include <string>

#include "Multithread/Component.hpp"
#include "Graphics/VertexType/Vertex3.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/ShaderData.hpp"

class SpriteRenderer : public Component {
public:
    static Vector3 vertices[4];
    static int indices[6];

    Buffer* verticeBuffer;
    Buffer* indiceBuffer;
    Texture* texture;
    Shader* shader;
    Pipeline* pipeline;
    ShaderData* shaderData;
    Vector4 uv[4];

    string texturePath;
    bool loaded = false;

    void LoadTexture(const std::string &textureLocation);
    void Load() override;
    void Update() override;
    void Render() override;
};

#endif