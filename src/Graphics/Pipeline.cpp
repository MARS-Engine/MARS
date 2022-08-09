#include "Pipeline.hpp"
#include "VEngine.hpp"
#include "Shader.hpp"
#include "CommandBuffer.hpp"
#include "Vulkan/VPipeline.hpp"

Pipeline::Pipeline(VEngine* _engine) {
    engine = _engine;
    pipeline = new VPipeline(engine->device, engine->swapchain, engine->renderPass);
}

void Pipeline::CreateLayout(size_t size) const {
    pipeline->CreateLayout(size);
}

void Pipeline::Create(Shader* shader, VertexInputDescription* description) const {
    pipeline->Create(shader->vShader, description);
}

void Pipeline::Bind(CommandBuffer* commandBuffer) const {
    pipeline->Bind(commandBuffer->vCommandBuffer);
}

void Pipeline::UpdateConstant(CommandBuffer* commandBuffer, void* data) const {
    pipeline->UpdateConstant(commandBuffer->vCommandBuffer, data);
}