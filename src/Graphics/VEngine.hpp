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

enum RENDER_TYPE {
    //Please avoid SIMPLE rendering, this should only be used if you want to write your own shaders. Because of the lighting system only deferred like renderers will be supported
    SIMPLE,
    DEFERRED
};

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

    RENDER_TYPE type;

    Camera* GetCamera();
    void Create(RENDER_TYPE type, Window* window);
    VkFramebuffer GetFramebuffer(int i);
    void PrepareDraw();
    void Draw();
    void Clean() const;
};

#endif