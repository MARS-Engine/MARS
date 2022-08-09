#ifndef __COMMANDBUFFER__
#define __COMMANDBUFFER__

#include "Vulkan/VCommandBuffer.hpp"
class VEngine;

class CommandBuffer {
public:
    VCommandBuffer* vCommandBuffer;
    VEngine* engine;

    CommandBuffer(VEngine* engine);

    void Create();
    void Reset() const;
    void Begin() const;
    void LoadDefault() const;
    void Draw(size_t vCount, size_t iCount) const;
    void End() const;
};

#endif