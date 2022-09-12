#ifndef __VENGINE__
#define __VENGINE__

#include "Vulkan/vtypes.hpp"
#include "command_buffer.hpp"
#include <map>

class window;
class vdevice;
class vinstance;
class vsurface;
class vswapchain;
class vcommand_pool;
class vrender_pass;
class vframebuffer;
class vsync;
class Camera;
class transform_3d;
class deferred_renderer;
class renderer_base;

enum RENDER_TYPE {
    MVRE_RENDERER_SIMPLE,
    MVRE_RENDERER_DEFERRED
};

enum RENDER_BACKEND {
    MVRE_BACKEND_VULKAN,
    MVRE_BACKEND_OPENGL
};

struct transparent_queue_item {
    float distance;
    VkCommandBuffer buffer;
};

class vengine {
private:
    void create_base();
public:
    static unsigned int frame_overlap;
    size_t render_frame = 0;
    window* surf_window;
    vdevice* device;
    vinstance* instance;
    vsurface* surface;
    vswapchain* swapchain;
    vcommand_pool* commandPool;
    vrender_pass* renderPass;
    vframebuffer* framebuffer;
    VmaAllocator allocator;
    vsync* sync;
    renderer_base* renderer;

    uint32_t image_index;

    std::vector<VkCommandBuffer> draw_queue;
    std::vector<transparent_queue_item> trans_queue;

    std::vector<Camera*> cameras;

    RENDER_TYPE type;
    RENDER_BACKEND backend;

    Camera* get_camera();
    void create(RENDER_BACKEND _backend, RENDER_TYPE _type, window* _window);
    VkFramebuffer get_framebuffer(int i);
    void prepare_draw();
    void draw();
    void clean() const;
};

#endif