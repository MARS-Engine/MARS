#include "renderer_base.hpp"
#include "../vengine.hpp"

renderer_base::renderer_base(vengine* _engine) {
    engine = _engine;
}

vrender_pass *renderer_base::get_render_pass() { return nullptr; }
void renderer_base::create_texture(const std::string& _name, deferred_texture_type _type) {}
VkFramebuffer renderer_base::get_framebuffer(size_t _index) { return nullptr; }
void renderer_base::clear() { }
void renderer_base::load() { }
void renderer_base::update() { }
void renderer_base::render() { }