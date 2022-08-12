#include "MeshRenderer.hpp"
#include "Math/Vector4.hpp"
#include "Math/Matrix4.hpp"
#include "Math/Matrix3.hpp"
#include "Math/Vector3.hpp"
#include "Time/TimeHelper.hpp"
#include "Graphics/Vulkan/VPipeline.hpp"
#include "Graphics/Vulkan/VTexture.hpp"
#include "Graphics/Vulkan/VShader.hpp"
#include "Graphics/Vulkan/VShaderData.hpp"
#include "Manager/LightManager.hpp"

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

void MeshRenderer::LoadTexture(const std::string &textureLocation) {
    if (!loaded) {
        texturePath = textureLocation;
        return;
    }

    if (texture == nullptr)
        texture = new Texture(GetEngine());

    texture->LoadTexture(textureLocation);
}

struct Push {
    Matrix4 model;
    Matrix4 normal;
};

void MeshRenderer::Load() {
    loaded = true;
    if (!meshPath.empty())
        LoadMesh(meshPath);

    if (!texturePath.empty())
        LoadTexture(texturePath);

    shader = new Shader(GetEngine());
    shader->LoadShader("Data/Shaders/Model.shader");

    auto desc = Vertex3().GetDescription();
    pipeline = new Pipeline(GetEngine(), shader);
    pipeline->CreateLayout();
    pipeline->ApplyInputDescription(&desc);
    pipeline->Create();

    shaderData = new ShaderData(shader, GetEngine());
    shaderData->GetUniform("MVP")->Generate(sizeof(Matrix4));
    shaderData->GetUniform("LIGHT")->Generate(sizeof(PointLightData));
    shaderData->GetUniform("push")->Generate(sizeof(Push));
    shaderData->GetUniform("texCoord")->SetTexture(texture);
    shaderData->Generate();
}
Vector3 camPos = { 0.f, 1.f, -2.f };

void MeshRenderer::Update() {
    if (verticeBuffer == nullptr)
        return;
    Matrix4 pv = Matrix4::PerspectiveFovLH(90.f, 1280.f, 720.f, 0.1f, 200.f) * Matrix4::LookAtLH(camPos, camPos + Vector3::Forward(), Vector3::Up());
    Matrix4 trans = transform()->GetTransform();
    Matrix4 mat = pv * trans;
    shaderData->GetUniform("MVP")->Update(&mat);

    Push push {
            trans,
            Matrix4::InverseTranspose(Matrix3(trans))
    };

    shaderData->GetUniform("LIGHT")->Update(&LightManager::pointLightData);
    shaderData->GetUniform("push")->Update(&push);
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