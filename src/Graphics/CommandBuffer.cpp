#include "CommandBuffer.hpp"
#include "VEngine.hpp"
#include "./Vulkan/VSwapchain.hpp"

CommandBuffer::CommandBuffer(VEngine* _engine) {
    engine = _engine;
}

VkCommandBuffer CommandBuffer::GetCommandBuffer() {
    return vCommandBuffer->rawCommandBuffers[engine->renderFrame];
}

void CommandBuffer::Create() {
    vCommandBuffer = new VCommandBuffer(engine->commandPool);
    vCommandBuffer->Create(VEngine::FRAME_OVERLAP);
}
void CommandBuffer::Reset() const {
    vCommandBuffer->Reset(engine->renderFrame);
}
void CommandBuffer::Begin() const {
    vCommandBuffer->Begin(engine->renderFrame);
}
void CommandBuffer::LoadDefault() const {
    vCommandBuffer->LoadDefault(engine->renderPass, engine->swapchain, engine->framebuffer, engine->imageIndex);
}
void CommandBuffer::Draw(size_t vCount, size_t iCount) const {
    vCommandBuffer->Draw(vCount, iCount);
}

void CommandBuffer::DrawIndexed(size_t size, size_t iCount) const {
    vCommandBuffer->DrawIndexed(size, iCount);
}

void CommandBuffer::End() const {
    vCommandBuffer->End();
}