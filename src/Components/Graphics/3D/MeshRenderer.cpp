#include "MeshRenderer.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Vector3.hpp"
#include "Time/TimeHelper.hpp"
#include "Manager/LightManager.hpp"
#include "Manager/ShaderManager.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Manager/PipelineManager.hpp"
#include "Components/Graphics/Camera.hpp"
#include "Graphics/Renderer/DeferredRenderer.hpp"

void MeshRenderer::LoadMesh(const string& meshLocation) {
    if (GetEngine() == nullptr) {
        meshPath = meshLocation;
        return;
    }

    if (mesh == nullptr)
        mesh = new Mesh();
    mesh->Load(meshLocation);

    if (verticeBuffer == nullptr) {
        verticeBuffer = new Buffer(GetEngine());
        verticeBuffer->Create(mesh->vertices.size() * sizeof(Vertex3), MEM_BUFF_VERTEX);
    }

    if (indiceBuffer == nullptr) {
        indiceBuffer = new Buffer(GetEngine());
        indiceBuffer->Create(mesh->indices.size() * sizeof(uint32_t), MEM_BUFF_INDEX);
    }

    //Need to resize verticeBuffer;
    verticeBuffer->Update(mesh->vertices.data());
    indiceBuffer->Update(mesh->indices.data());
}

void MeshRenderer::LoadTexture(const std::string &textureLocation) {
    if (GetEngine() == nullptr) {
        texturePath = textureLocation;
        return;
    }

    if (texture == nullptr)
        texture = new Texture(GetEngine());

    texture->LoadTexture(textureLocation);
}

void MeshRenderer::Load() {
    model = { };

    isRenderer = true;
    if (!meshPath.empty())
        LoadMesh(meshPath);

    if (!texturePath.empty())
        LoadTexture(texturePath);

    shader = ShaderManager::GetShader("Engine/Assets/Shaders/Model.shader", GetEngine());

    pipeline = PipelineManager::GetPipeline("Model");

    if (pipeline == nullptr) {
        auto desc = Vertex3::GetDescription();
        pipeline = new Pipeline(GetEngine(), shader, RenderPassManager::GetRenderPass("Renderer", GetEngine()));
        pipeline->CreateLayout();
        pipeline->ApplyInputDescription(&desc);
        pipeline->Create();
        PipelineManager::AddPipeline("Model", pipeline);
    }

    shaderData = new ShaderData(shader, GetEngine());
    shaderData->GetUniform("Model")->Generate(sizeof(ShaderModel));
    shaderData->GetUniform("Material")->Generate(sizeof(MaterialData));
    shaderData->GetUniform("texCoord")->SetTexture(texture);
    shaderData->Generate();

    GetCommandBuffer()->renderPass = RenderPassManager::GetRenderPass("Renderer", GetEngine());
}

void MeshRenderer::Update() {
    if (verticeBuffer == nullptr)
        return;

    Matrix4 pv = GetEngine()->GetCamera()->ProjectionView;
    Matrix4 trans = transform()->GetTransform();
    model.model = trans;
    model.mvp = pv * trans;

    shaderData->GetUniform("Model")->Update(&model);
    shaderData->GetUniform("Material")->Update(&GetMaterial()->data);
}

void MeshRenderer::PreRender() {
    if (verticeBuffer == nullptr)
        return;

    pipeline->Bind(GetCommandBuffer());
    verticeBuffer->Bind(GetCommandBuffer());
    indiceBuffer->Bind(GetCommandBuffer());
    shaderData->Bind(GetCommandBuffer(), pipeline);
    GetCommandBuffer()->DrawIndexed(mesh->indices.size(), 1);
}