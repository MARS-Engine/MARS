#ifndef __BUFFER__
#define __BUFFER__

#include <string>

class CommandBuffer;
class VBuffer;
class VEngine;

#include "Vulkan/VTypes.hpp"

enum BufferType {
    MEM_BUFF_VERTEX,
    MEM_BUFF_INDEX
};

class Buffer {
public:
    VBuffer* stagingBuffer;
    VBuffer* vBuffer;
    VEngine* engine;
    BufferType type;

    Buffer(VEngine* engine);

    void Create(size_t size, BufferType type);
    void Update(void* data) const;
    void Bind(CommandBuffer* commandBuffer) const;
};

#endif