#include "Pipeline.hpp"
#include "VEngine.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "Vulkan/VPipeline.hpp"

Pipeline::Pipeline(VEngine* _engine, Shader* shader) {
    engine = _engine;
    pipeline = new VPipeline(shader->vShader, engine->device, engine->swapchain, engine->renderPass);
}

Pipeline::Pipeline(VEngine* _engine, Shader* shader, VRenderPass* renderPass) {
    engine = _engine;
    pipeline = new VPipeline(shader->vShader, engine->device, engine->swapchain, renderPass);
}

void Pipeline::CreateLayout(size_t size) const {
    pipeline->CreateLayout(size);
}

void Pipeline::ApplyInputDescription(VertexInputDescription* description) const {
    pipeline->ApplyInputDescription(description);
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