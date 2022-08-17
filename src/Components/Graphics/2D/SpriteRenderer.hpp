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

struct Sprite {
private:
    Vector2 textureSize;
public:
    Vector4 uv;
    Sprite(Vector2 size, Vector2 offset, Vector2 textureSize);
    void SetOffset(Vector2 offset);
};

class SpriteRenderer : public Component {
private:
    Sprite* sprite;
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

    void SetSprite(Sprite* sprite);
    void LoadTexture(const std::string &textureLocation);
    void Load() override;
    void Update() override;
    void PreRender() override;
};

#endif