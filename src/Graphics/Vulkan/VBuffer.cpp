#include "VBuffer.hpp"
#include "VDevice.hpp"
#include "VInstance.hpp"
#include "VCommandBuffer.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VBuffer::VBuffer(VmaAllocator& _allocator) {
    allocator = _allocator;
}

VmaAllocator VBuffer::GenerateAllocator(VDevice* device, VInstance* instance) {
    VmaAllocator _allocator;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = device->physicalDevice;
    allocatorInfo.device = device->rawDevice;
    allocatorInfo.instance = instance->rawInstance;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
    return _allocator;
}

void VBuffer::Create(size_t size, VkBufferUsageFlagBits usage, VmaMemoryUsage memoryUsage) {
    bufferSize = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, nullptr));
}

void VBuffer::Update(void* data) {
    void* tempData;
    vmaMapMemory(allocator, allocation, &tempData);
    memcpy(tempData, data, bufferSize);
    vmaUnmapMemory(allocator, allocation);
}

void VBuffer::Bind(VCommandBuffer* commandBuffer) {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], 0, 1, &buffer, &offset);
}