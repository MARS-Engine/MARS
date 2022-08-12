#ifndef __VENGINE__
#define __VENGINE__

#include "Vulkan/VTypes.hpp"
#include "CommandBuffer.hpp"

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

class VEngine {
public:
    static constexpr unsigned int FRAME_OVERLAP = 2;
    size_t renderFrame = 0;
    Window* window;
    VDevice* device;
    VInstance* instance;
    VSurface* surface;
    VSwapchain* swapchain;
    VCommandPool* commandPool;
    CommandBuffer* commandBuffer;
    VRenderPass* renderPass;
    VFramebuffer* framebuffer;
    VmaAllocator allocator;
    VSync* sync;

    uint32_t imageIndex;

    vector<Camera*> cameras;

    Camera* GetCamera();
	void Create(Window* window);
    void PrepareDraw();
    void Draw();
    void Clean() const;
};

#endif