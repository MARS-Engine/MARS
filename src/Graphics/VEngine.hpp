#ifndef __VENGINE__
#define __VENGINE__

#include "Vulkan/VTypes.hpp"
#include "CommandBuffer.hpp"
#include <map>

class Window;
class VDevice;
class VInstance;
class VSurface;
class VSwapchain;
class VCommandPool;
class VRenderPass;
class VFramebuffer;
class VSync;
class Camera;
class Transform;
class DeferredRenderer;
class RendererBase;

class VEngine {
private:
    void CreateBase();
public:
    static unsigned int FRAME_OVERLAP;
    size_t renderFrame = 0;
    Window* window;
    VDevice* device;
    VInstance* instance;
    VSurface* surface;
    VSwapchain* swapchain;
    VCommandPool* commandPool;
    VRenderPass* renderPass;
    VFramebuffer* framebuffer;
    VmaAllocator allocator;
    VSync* sync;
    RendererBase* renderer;

    uint32_t imageIndex;

    vector<VkCommandBuffer> drawQueue;
    map<float, VkCommandBuffer, std::greater<>> transQueue;

    vector<Camera*> cameras;

    Camera* GetCamera();
    void Create(Window* window);
    void CreateDeferred(Window* window);
    void PrepareDraw();
    void Draw();
    void Clean() const;
};

#endif