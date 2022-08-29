#ifndef __DEFERRED__HANDLER__
#define __DEFERRED__HANDLER__

#include "Manager/pipeline_manager.hpp"
#include "Manager/render_pass_manager.hpp"
#include "Manager/shader_manager.hpp"
#include "../shader_data.hpp"
#include "../Vulkan/vtypes.hpp"

#include "renderer_base.hpp"
#include "../Vulkan/vframebuffer.hpp"

class deferred_renderer : public  renderer_base {
public:
    shader_data* data;
    vector<texture*> textures_data;
    vector<deferred_texture> textures;
    vframebuffer* framebuffer;

    using renderer_base::renderer_base;

    void create_texture(const string& _name, deferred_texture_type _type) override;
    vrender_pass* get_render_pass() override;
    VkFramebuffer get_framebuffer(size_t _index) override;
    void clear() override;
    void load() override;
    void update() override;
    void render() override;
};

#endif