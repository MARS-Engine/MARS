#ifndef __RENDERER__BASE__
#define __RENDERER__BASE__

#include "Graphics/command_buffer.hpp"
#include "Graphics/texture.hpp"
#include "Graphics/Vulkan/vrender_pass.hpp"

class vengine;

enum deferred_texture_type {
    COLOR,
    POSITION
};

struct deferred_texture {
public:
    std::string name;
    deferred_texture_type type;
    texture* _texture;
};

class renderer_base {
public:
    command_buffer* clear_buffer = nullptr;
    command_buffer* render_buffer = nullptr;
    vengine* engine = nullptr;

    explicit renderer_base(vengine* _engine);

    virtual void create_texture(const std::string& _name, deferred_texture_type _type);
    virtual vrender_pass* get_render_pass();
    virtual VkFramebuffer get_framebuffer(size_t _index);
    virtual void clear();
    virtual void load();
    virtual void update();
    virtual void render();
};

#endif