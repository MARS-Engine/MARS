#include "SpriteRenderer.hpp"
#include "Manager/ShaderManager.hpp"
#include "Manager/PipelineManager.hpp"
#include "Components/Graphics/Camera.hpp"
#include "Manager/MaterialManager.hpp"
#include "Manager/LightManager.hpp"
#include "Math/Matrix3.hpp"
#include "Time/TimeHelper.hpp"
#include <math.h>

Vector3 SpriteRenderer::vertices[4] = {
        { -.5f,  .5f, -1.0f },
        {  .5f,  .5f, -1.0f },
        { -.5f, -.5f, -1.0f },
        {  .5f, -.5f, -1.0f }
};

int SpriteRenderer::indices[6] = { 0, 1, 2, 1, 3, 2 };

struct SceneData {
    Matrix4 mvp;
    Matrix4 model;
    Matrix4 mv;
    Matrix4 normal;
    Vector4 camPos;
};

void SpriteRenderer::Load() {
    Component::Load();

    isRenderer = true;

    material = MaterialManager::GetMaterial("Sprite");
    material->enableTransparency = true;

    verticeBuffer = new Buffer(GetEngine());
    verticeBuffer->Create(sizeof(Vertex3) * 4, MEM_BUFF_VERTEX);

    indiceBuffer = new Buffer(GetEngine());
    indiceBuffer->Create(sizeof(uint32_t) * 6, MEM_BUFF_INDEX);

    verticeBuffer->Update(&vertices);
    indiceBuffer->Update(&indices);

    shader = ShaderManager::GetShader("Engine/Assets/Shaders/Sprite.shader", GetEngine());

    pipeline = PipelineManager::GetPipeline("SpritePipeline");

    if (pipeline == nullptr) {
        auto desc = Vector3::GetDescription();
        pipeline = new Pipeline(GetEngine(), shader);
        pipeline->CreateLayout();
        pipeline->ApplyInputDescription(&desc);
        pipeline->Create();
        PipelineManager::AddPipeline("SpritePipeline", pipeline);
    }

    shaderData = new ShaderData(shader, GetEngine());
    shaderData->GetUniform("UV")->Generate(sizeof(Quad));
    shaderData->GetUniform("Model")->Generate(sizeof(Matrix4));
    shaderData->GetUniform("texCoord")->SetTexture(sprite->texture);
    shaderData->Generate();

    last_texture = sprite->texture;

    SetSprite(sprite);
}

void SpriteRenderer::SetSprite(Sprite* _sprite) {
    sprite = _sprite;
    if (!loaded)
        return;

    uv = sprite->GetUV();

    if (last_texture != sprite->texture) {
        shaderData->ChangeTexture("texCoord", sprite->texture);
        last_texture = sprite->texture;
    }

    shaderData->GetUniform("UV")->Update(&uv);
    object->ExecuteCode(PRE_RENDER);
}

void SpriteRenderer::Update() {
    Matrix4 mvp = GetEngine()->GetCamera()->ProjectionView * transform()->GetTransform();
    shaderData->GetUniform("Model")->Update(&mvp);
}

void SpriteRenderer::PreRender() {
    pipeline->Bind(GetCommandBuffer());
    verticeBuffer->Bind(GetCommandBuffer());
    indiceBuffer->Bind(GetCommandBuffer());
    shaderData->Bind(GetCommandBuffer(), pipeline);
    GetCommandBuffer()->DrawIndexed(6, 1);
}