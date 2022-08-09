#ifndef __BUFFER__
#define __BUFFER__

#include <string>

class CommandBuffer;
class VBuffer;
class VEngine;

#include "vulkan/VTypes.hpp"

class Buffer {
public:
    VBuffer* vBuffer;
    VEngine* engine;

    Buffer(VEngine* engine);

    void Create(size_t size, VkBufferUsageFlagBits usage, VmaMemoryUsage memoryUsage);
    void Update(void* data);
    void Bind(CommandBuffer* commandBuffer);
};

#endif