#ifndef __COMMAND__BUFFER__
#define __COMMAND__BUFFER__

#include "Vulkan/VTypes.hpp"
#include "Vulkan/VCommandBuffer.hpp"
class VEngine;

class CommandBuffer {
public:
    VCommandBuffer* vCommandBuffer;
    VEngine* engine;
    VRenderPass* renderPass;

    CommandBuffer(VEngine* engine);

    VkCommandBuffer  GetCommandBuffer();

    void Create();
    void Reset() const;
    void Begin() const;
    void Begin(size_t index) const;
    void LoadDefault() const;
    void LoadDefault(size_t index) const;
    void LoadDefault(size_t index, VFramebuffer* framebuffer) const;
    void Draw(size_t vCount, size_t iCount) const;
    void DrawIndexed(size_t size, size_t iCount) const;
    void End() const;
};

#endif