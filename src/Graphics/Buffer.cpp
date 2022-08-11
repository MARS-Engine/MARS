#include "Buffer.hpp"
#include "VEngine.hpp"
#include "Vulkan/VBuffer.hpp"
#include "Vulkan/VCommandBuffer.hpp"

Buffer::Buffer(VEngine* _engine) {
    engine = _engine;
    stagingBuffer = new VBuffer(engine->device, engine->allocator);
    vBuffer = new VBuffer(engine->device, engine->allocator);
}

void Buffer::Create(size_t size, VkBufferUsageFlagBits usage) const {
    stagingBuffer->Create(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    vBuffer->Create(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
}

void Buffer::Update(void* data) const {
    stagingBuffer->Update(data);
    vBuffer->CopyBuffer(stagingBuffer);
}

void Buffer::Bind(CommandBuffer* commandBuffer) const {
    vBuffer->Bind(commandBuffer->vCommandBuffer);
}