#ifndef __SPRITE__RENDERER__
#define __SPRITE__RENDERER__

#include <string>

#include "Multithread/Component.hpp"
#include "Graphics/VertexType/Vertex3.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Pipeline.hpp"
#include "Graphics/ShaderData.hpp"
#include "Type/Sprite.hpp"

struct SpriteRendererData {
    unsigned int flipX;
    unsigned int flipY;
};

class SpriteRenderer : public Component {
private:
    Texture* last_texture = nullptr;
    Sprite* sprite = nullptr;
public:
    static Vector3 vertices[4];
    static int indices[6];

    Buffer* verticeBuffer = nullptr;
    Buffer* indiceBuffer = nullptr;
    Pipeline* pipeline = nullptr;
    ShaderData* shaderData = nullptr;
    Quad uv;
    SpriteRendererData rendererData{};

    void SetSprite(Sprite* sprite);
    void Load() override;
    void Update() override;
    void PreRender() override;
};

#endif