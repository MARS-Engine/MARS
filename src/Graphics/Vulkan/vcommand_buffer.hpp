#ifndef __VCOMMAND__BUFFER__
#define __VCOMMAND__BUFFER__

#include "vtypes.hpp"
#include "Math/vector4.hpp"
#include <vector>

using namespace std;

class vcommand_pool;
class vrender_pass;
class vswapchain;
class vframebuffer;

class vcommand_buffer {
public:
    vector<VkCommandBuffer> raw_command_buffers;
    vcommand_pool* command_pool;
    size_t record_index = 0;
    vector4 clear_color;

    vcommand_buffer(vcommand_pool* _commandPool);

    void create(size_t _size);
    void reset(size_t _index);
    void begin(size_t _index);
    void load_default(vrender_pass* _render_pass, vswapchain* _swapchain, VkFramebuffer _framebuffer, size_t _image_index);
    void draw(size_t _v_count, size_t _i_count);
    void draw_indexed(size_t _size, size_t _i_count);
    void end();
    void end_single();
};

#endif