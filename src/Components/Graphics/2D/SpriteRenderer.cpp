#include "SpriteRenderer.hpp"
#include "Manager/ShaderManager.hpp"
#include "Manager/PipelineManager.hpp"
#include "Components/Graphics/Camera.hpp"
#include "Manager/MaterialManager.hpp"
#include "Manager/LightManager.hpp"
#include "Math/Matrix3.hpp"
#include "Time/TimeHelper.hpp"
#include "Manager/RenderPassManager.hpp"
#include <math.h>
#include "Graphics/Renderer/DeferredRenderer.hpp"
#include "Graphics/Window.hpp"

Vector3 SpriteRenderer::vertices[4] = {
        { -.5f,  .5f, .0f },
        {  .5f,  .5f, .0f },
        { -.5f, -.5f, .0f },
        {  .5f, -.5f, .0f }
};

int SpriteRenderer::indices[6] = { 0, 1, 2, 1, 3, 2 };

struct Model {
    Matrix4 mvp;
    Matrix4 model;
};

void SpriteRenderer::Load() {
    Component::Load();

    isRenderer = true;

    if (GetMaterial()->name == "default") {
        SetMaterial(MaterialManager::GetMaterial("Sprite"));
        GetMaterial()->enableTransparency = true;
        GetMaterial()->shader = ShaderManager::GetShader("Engine/Assets/Shaders/Sprite.shader", GetEngine());
    }

    verticeBuffer = new Buffer(GetEngine());
    verticeBuffer->Create(sizeof(Vertex3) * 4, MEM_BUFF_VERTEX);

    indiceBuffer = new Buffer(GetEngine());
    indiceBuffer->Create(sizeof(uint32_t) * 6, MEM_BUFF_INDEX);

    verticeBuffer->Update(&vertices);
    indiceBuffer->Update(&indices);

    pipeline = PipelineManager::GetPipeline("SpritePipeline" + GetMaterial()->shader->shaderPath);

    if (pipeline == nullptr) {
        auto desc = Vector3::GetDescription();
        pipeline = new Pipeline(GetEngine(), GetMaterial()->shader, RenderPassManager::GetRenderPass("Renderer", GetEngine()));
        pipeline->CreateLayout();
        pipeline->ApplyViewport({ .size = GetEngine()->window->size });
        pipeline->ApplyInputDescription(&desc);
        pipeline->Create();
        PipelineManager::AddPipeline("SpritePipeline" + GetMaterial()->shader->shaderPath, pipeline);
    }

    shaderData = new ShaderData(GetMaterial()->shader, GetEngine());
    shaderData->GetUniform("UV")->Generate(sizeof(Quad));
    shaderData->GetUniform("Model")->Generate(sizeof(Model));
    shaderData->GetUniform("texCoord")->SetTexture(sprite->texture);
    shaderData->GetUniform("__SPRITE_RENDERER")->Generate(sizeof(SpriteRendererData));
    shaderData->Generate();

    last_texture = sprite->texture;

    SetSprite(sprite);

    GetCommandBuffer()->renderPass = RenderPassManager::GetRenderPass("Renderer", GetEngine());
}

void SpriteRenderer::SetSprite(Sprite* _sprite) {
    sprite = _sprite;
    if (!loaded)
        return;

    uv = sprite->GetUV();

    if (last_texture != sprite->texture) {
        shaderData->ChangeTexture("texCoord", sprite->texture);
        last_texture = sprite->texture;
        object->ExecuteCode(PRE_RENDER);
    }

    shaderData->GetUniform("UV")->Update(&uv);
}

void SpriteRenderer::Update() {
    Model model;
    model.model = transform()->GetTransform();
    model.mvp = GetEngine()->GetCamera()->ProjectionView * model.model;

    shaderData->GetUniform("Model")->Update(&model);
    shaderData->GetUniform("__SPRITE_RENDERER")->Update(&rendererData);
}

void SpriteRenderer::PreRender() {
    pipeline->Bind(GetCommandBuffer());
    verticeBuffer->Bind(GetCommandBuffer());
    indiceBuffer->Bind(GetCommandBuffer());
    shaderData->Bind(GetCommandBuffer(), pipeline);
    GetCommandBuffer()->DrawIndexed(6, 1);
}