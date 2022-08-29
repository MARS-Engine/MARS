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
    //Please avoid SIMPLE rendering, this should only be used if you want to write your own shaders. Because of the lighting system only deferred like renderers will be supported
    SIMPLE,
    DEFERRED
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

    vector<VkCommandBuffer> draw_queue;
    vector<transparent_queue_item> trans_queue;

    vector<Camera*> cameras;

    RENDER_TYPE type;

    Camera* get_camera();
    void create(RENDER_TYPE _type, window* _window);
    VkFramebuffer get_framebuffer(int i);
    void prepare_draw();
    void draw();
    void clean() const;
};

#endif