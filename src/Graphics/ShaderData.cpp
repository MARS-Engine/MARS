#include "ShaderData.hpp"
#include "VEngine.hpp"
#include "Texture.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"
#include "Vulkan/VShaderData.hpp"
#include "Vulkan/VShader.hpp"

Uniform::Uniform(VUniform* uni, VEngine* _engine) {
    engine = _engine;
    uniform = uni;
}

void Uniform::Generate(size_t bufferSize) {
    uniform->Generate(bufferSize, VEngine::FRAME_OVERLAP);
}

void Uniform::Update(void* data) {
    uniform->Update(data, engine->renderFrame);
}
void Uniform::SetTexture(Texture* texture) {
    uniform->SetTexture(texture->vTexture);
}

ShaderData::ShaderData(Shader* _shader, VEngine* _engine) {
    shader = _shader;
    engine = _engine;

    shaderData = new VShaderData(shader->vShader, engine->allocator);
    for (auto uni : shaderData->uniforms)
        uniforms.push_back(new Uniform(uni, engine));
}

Uniform* ShaderData::GetUniform(string name) {
    for (auto uni : uniforms)
        if (uni->uniform->data->name == name)
            return uni;
    Debug::Error("Uniform - Faield to find uniform with name - " + name);
    return nullptr;
}

void ShaderData::Bind(CommandBuffer* commandBuffer, Pipeline* pipeline) {
    shaderData->Bind(commandBuffer->vCommandBuffer, pipeline->pipeline);
}

void ShaderData::Generate() {
    shaderData->Generate(VEngine::FRAME_OVERLAP);
}