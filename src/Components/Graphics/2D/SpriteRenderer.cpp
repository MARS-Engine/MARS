#include "SpriteRenderer.hpp"
#include "Manager/ShaderManager.hpp"
#include "Manager/PipelineManager.hpp"
#include "Components/Graphics/Camera.hpp"
#include "Manager/MaterialManager.hpp"

Vector3 SpriteRenderer::vertices[4] = {
    { -.5f,  .5f, -1.0f },
    {  .5f,  .5f, -1.0f },
    { -.5f, -.5f, -1.0f },
    {  .5f, -.5f, -1.0f }
};

int SpriteRenderer::indices[6] = { 0, 1, 2, 1, 3, 2 };

void SpriteRenderer::LoadTexture(const std::string &textureLocation) {
    if (!loaded) {
        texturePath = textureLocation;
        return;
    }

    if (texture == nullptr)
        texture = new Texture(GetEngine());

    texture->LoadTexture(textureLocation);
}

void SpriteRenderer::Load() {
    loaded = true;
    if (texturePath != "") {
        LoadTexture(texturePath);
    }

    object->material = MaterialManager::GetMaterial("sprite");
    object->material->enableTransparency = true;

    verticeBuffer = new Buffer(GetEngine());
    verticeBuffer->Create(sizeof(Vertex3) * 4, MEM_BUFF_VERTEX);

    indiceBuffer = new Buffer(GetEngine());
    indiceBuffer->Create(sizeof(uint32_t) * 6, MEM_BUFF_INDEX);

    verticeBuffer->Update(&vertices);
    indiceBuffer->Update(&indices);

    shader = ShaderManager::GetShader("Data/Shaders/Sprite.shader", GetEngine());

    pipeline = PipelineManager::GetPipeline("SpritePipeline");

    if (pipeline == nullptr) {
        auto desc = Vector3::GetDescription();
        pipeline = new Pipeline(GetEngine(), shader);
        pipeline->CreateLayout();
        pipeline->ApplyInputDescription(&desc);
        pipeline->Create();
        PipelineManager::AddPipeline("SpritePipeline", pipeline);
    }

    uv[0] = { 0.0, 0.0, 0.0, 0.0 };
    uv[1] = { 1.0, 0.0, 0.0, 0.0 };
    uv[2] = { 0.0, 1.0, 0.0, 0.0 };
    uv[3] = { 1.0, 1.0, 0.0, 0.0 };

    shaderData = new ShaderData(shader, GetEngine());
    shaderData->GetUniform("MVP")->Generate(sizeof(Matrix4));
    shaderData->GetUniform("UV")->Generate(sizeof(Vector4) * 4);
    shaderData->GetUniform("texCoord")->SetTexture(texture);
    shaderData->Generate();
}

void SpriteRenderer::Update() {
    Matrix4 pv =GetEngine()->GetCamera()->ProjectionView;
    Matrix4 trans = transform()->GetTransform();
    Matrix4 mat = pv * trans;
    shaderData->GetUniform("MVP")->Update(&mat);
    shaderData->GetUniform("UV")->Update(&uv);
}

void SpriteRenderer::PreRender() {
    pipeline->Bind(GetCommandBuffer());
    verticeBuffer->Bind(GetCommandBuffer());
    indiceBuffer->Bind(GetCommandBuffer());
    shaderData->Bind(GetCommandBuffer(), pipeline);
    GetCommandBuffer()->DrawIndexed(6, 1);
}