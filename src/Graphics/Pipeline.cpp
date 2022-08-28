#include "Pipeline.hpp"
#include "VEngine.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "Vulkan/VPipeline.hpp"

Pipeline::Pipeline(VEngine* _engine, Shader* shader) {
    engine = _engine;
    pipeline = new VPipeline(engine->device);
    pipeline->LoadShader(shader->vShader);
    pipeline->LoadRenderPass(engine->renderPass);
}

Pipeline::Pipeline(VEngine* _engine, Shader* shader, VRenderPass* renderPass) {
    engine = _engine;
    pipeline = new VPipeline(engine->device);
    pipeline->LoadShader(shader->vShader);
    pipeline->LoadRenderPass(renderPass);
}

void Pipeline::CreateLayout() const {
    pipeline->CreateLayout();
}

void Pipeline::ApplyInputDescription(VertexInputDescription* description) const {
    pipeline->ApplyInputDescription(description);
}

void Pipeline::ApplyViewport(PipelineViewport viewport) {
    pipeline->ApplyViewport(viewport);
}

void Pipeline::Create() const {
    pipeline->Create();
}

void Pipeline::Bind(CommandBuffer* commandBuffer) const {
    pipeline->Bind(commandBuffer->vCommandBuffer);
}

void Pipeline::UpdateConstant(CommandBuffer* commandBuffer, void* data) const {
    pipeline->UpdateConstant(commandBuffer->vCommandBuffer, data);
}