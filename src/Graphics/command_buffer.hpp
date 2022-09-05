#ifndef __COMMAND__BUFFER__
#define __COMMAND__BUFFER__

#include "Vulkan/vtypes.hpp"
#include "Vulkan/vcommand_buffer.hpp"
class vengine;

class command_buffer {
public:
    vcommand_buffer* base_command_buffer;
    vengine* engine;
    vrender_pass* render_pass;

    command_buffer(vengine* _engine);

    VkCommandBuffer get_command_buffer();

    void create();
    void reset() const;
    void begin() const;
    void begin(size_t _index) const;
    void load_default() const;
    void load_default(size_t _index) const;
    void load_default(size_t _index, vframebuffer* _framebuffer) const;
    void draw(size_t _v_count, size_t _i_count) const;
    void draw_indexed(size_t _size, size_t _i_count) const;
    void end() const;
};

#endif