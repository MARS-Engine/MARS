#include "Buffer.hpp"
#include "VEngine.hpp"
#include "Vulkan/VBuffer.hpp"
#include "Vulkan/VCommandBuffer.hpp"

Buffer::Buffer(VEngine* _engine) {
    engine = _engine;
    vBuffer = new VBuffer(engine->allocator);
}

void Buffer::Create(size_t size, VkBufferUsageFlagBits usage, VmaMemoryUsage memoryUsage) {
    vBuffer->Create(size, usage, memoryUsage);
}

void Buffer::Update(void* data) {
    vBuffer->Update(data);
}

void Buffer::Bind(CommandBuffer* commandBuffer) {
    vBuffer->Bind(commandBuffer->vCommandBuffer);
}