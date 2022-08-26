#ifndef __VCOMMAND__BUFFER__
#define __VCOMMAND__BUFFER__

#include "VTypes.hpp"
#include "Math/Vector4.hpp"
#include <vector>

using namespace std;

class VCommandPool;
class VRenderPass;
class VSwapchain;
class VFramebuffer;

class VCommandBuffer {
public:
    vector<VkCommandBuffer> rawCommandBuffers;
    VCommandPool* commandPool;
    size_t recordIndex = 0;
    Vector4 clearColor;

    VCommandBuffer(VCommandPool* commandPool);

    void Create(size_t size);
    void Reset(size_t index);
    void Begin(size_t index);
    void LoadDefault(VRenderPass* renderPass, VSwapchain* swapchain, VkFramebuffer framebuffer, size_t imageIndex);
    void Draw(size_t vCount, size_t iCount);
    void DrawIndexed(size_t size, size_t iCount);
    void End();
    void EndSingle();
};

#endif