#ifndef __BUFFER__
#define __BUFFER__

#include <string>

class CommandBuffer;
class VBuffer;
class VEngine;

#include "Vulkan/VTypes.hpp"

class Buffer {
public:
    VBuffer* stagingBuffer;
    VBuffer* vBuffer;
    VEngine* engine;

    Buffer(VEngine* engine);

    void Create(size_t size, VkBufferUsageFlagBits usage) const;
    void Update(void* data) const;
    void Bind(CommandBuffer* commandBuffer) const;
};

#endif