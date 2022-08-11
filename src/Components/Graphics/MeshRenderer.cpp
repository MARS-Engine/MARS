#include "MeshRenderer.hpp"
#include "Math/Vector4.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Vector3.hpp"
#include "Time/TimeHelper.hpp"
#include "Graphics/Vulkan/VPipeline.hpp"
#include "Graphics/Vulkan/VTexture.hpp"
#include "Graphics/Vulkan/VShader.hpp"
#include "Graphics/Vulkan/VShaderData.hpp"

void MeshRenderer::LoadMesh(const string& meshLocation) {
    if (!loaded) {
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

void MeshRenderer::Load() {
    loaded = true;
    if (!meshPath.empty())
        LoadMesh(meshPath);

    shader = new Shader(GetEngine());
    shader->LoadShader("Data/Shaders/Model.shader");

    auto desc = Vertex3().GetDescription();
    pipeline = new Pipeline(GetEngine(), shader);
    pipeline->CreateLayout();
    pipeline->ApplyInputDescription(&desc);
    pipeline->Create();

    texture = new Texture(GetEngine());
    texture->LoadTexture("Data/Texture/Astroid/asteroid_base_color.jpg");

    shaderData = new ShaderData(shader, GetEngine());
    shaderData->GetUniform("MVP")->Generate(sizeof(Matrix4));
    shaderData->GetUniform("texCoord")->SetTexture(texture);
    shaderData->Generate();

    //uniform = new Uniform(GetEngine());
    //uniform->Create(sizeof(GPUCameraData), shader);
}
Vector3 camPos = { 0.f, 0.f, -2.f };

void MeshRenderer::Update() {
    if (verticeBuffer == nullptr)
        return;
    Matrix4 mat = Matrix4::PerspectiveFovLH(90.f, 1280.f, 720.f, 0.1f, 200.f) * Matrix4::LookAtLH(camPos, camPos + Vector3::Forward(), Vector3::Up()) * transform()->GetTransform();
    shaderData->GetUniform("MVP")->Update(&mat);
    //uniform->Update(&camData);
}

void MeshRenderer::Render() {
    if (verticeBuffer == nullptr)
        return;

    pipeline->Bind(GetEngine()->commandBuffer);
    verticeBuffer->Bind(GetEngine()->commandBuffer);
    indiceBuffer->Bind(GetEngine()->commandBuffer);
    shaderData->Bind(GetEngine()->commandBuffer, pipeline);
    GetEngine()->commandBuffer->DrawIndexed(mesh->indices.size(), 1);
}