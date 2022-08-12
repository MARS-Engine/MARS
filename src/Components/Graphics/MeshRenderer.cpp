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

struct PointLight {
    Vector4 pos;
    Vector4 color;
};

struct Push {
    Matrix4 model;
    Matrix4 normal;
};


struct LIGHT {
    Matrix4 pv;
    Vector4 ambient;
    PointLight lights[2];
    int lightSize;
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
    shaderData->GetUniform("LIGHT")->Generate(sizeof(LIGHT));
    shaderData->GetUniform("push")->Generate(sizeof(Push));
    shaderData->GetUniform("texCoord")->SetTexture(texture);
    shaderData->Generate();
}
Vector3 camPos = { 0.f, 1.f, -2.f };

void MeshRenderer::Update() {
    if (verticeBuffer == nullptr)
        return;
    Matrix4 pv = Matrix4::PerspectiveFovLH(90.f, 1280.f, 720.f, 0.1f, 200.f) * Matrix4::LookAtLH(camPos, camPos + Vector3::Forward(), Vector3::Up());
    Matrix4 t = transform()->GetTransform();
    Matrix4 mat = pv * t;
    shaderData->GetUniform("MVP")->Update(&mat);
    LIGHT light{};
    light.pv = pv;
    light.ambient = (1.0f, 1.0f, 1.0f, .02f);
    light.lights[0] = { Vector4(2.0f, 1.0f, -1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) };
    light.lights[1] = { Vector4(-2.0f, 1.0f, 1.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f) };
    light.lightSize = 2;

    Push dw {
        t,
        Matrix4::InverseTranspose(Matrix3(t))
    };

    shaderData->GetUniform("LIGHT")->Update(&light);
    shaderData->GetUniform("push")->Update(&dw);
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