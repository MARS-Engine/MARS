#ifndef __VBUFFER__
#define __VBUFFER__

#include "VTypes.hpp"

class VDevice;
class VInstance;
class VCommandBuffer;

class VBuffer {
public:
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocator allocator;
    size_t bufferSize;

    VBuffer(VmaAllocator& allocator);

    static VmaAllocator GenerateAllocator(VDevice* device, VInstance* instance);

    void Create(size_t size, VkBufferUsageFlagBits usage, VmaMemoryUsage memoryUsage);
    void Update(void* data);
    void Bind(VCommandBuffer* commandBuffer);
};

#endif