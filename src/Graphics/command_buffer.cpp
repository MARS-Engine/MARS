#include "command_buffer.hpp"
#include "vengine.hpp"
#include "./Vulkan/vswapchain.hpp"
#include "Manager/render_pass_manager.hpp"
#include "Vulkan/vframebuffer.hpp"

command_buffer::command_buffer(vengine* _engine) {
    engine = _engine;
    render_pass = render_pass_manager::get_render_pass("Renderer", engine);
}

VkCommandBuffer command_buffer::get_command_buffer() {
    return base_command_buffer->raw_command_buffers[engine->render_frame];
}

void command_buffer::create() {
    base_command_buffer = new vcommand_buffer(engine->commandPool);
    base_command_buffer->create(vengine::frame_overlap);
}

void command_buffer::reset() const {
    base_command_buffer->reset(engine->render_frame);
}

void command_buffer::begin() const {
    base_command_buffer->begin(engine->render_frame);
}

void command_buffer::begin(size_t _index) const {
    base_command_buffer->begin(_index);
}

void command_buffer::load_default() const {
    base_command_buffer->load_default(render_pass, engine->swapchain,
                                      engine->get_framebuffer(base_command_buffer->record_index), engine->image_index);
}

void command_buffer::load_default(size_t _index) const {
    base_command_buffer->load_default(render_pass, engine->swapchain,
                                      engine->get_framebuffer(base_command_buffer->record_index), _index);
}

void command_buffer::load_default(size_t _index, vframebuffer* _framebuffer) const {
    base_command_buffer->load_default(render_pass, engine->swapchain, _framebuffer->raw_framebuffers[_index], _index);
}

void command_buffer::draw(size_t _v_count, size_t _i_count) const {
    base_command_buffer->draw(_v_count, _i_count);
}

void command_buffer::draw_indexed(size_t _size, size_t _i_count) const {
    base_command_buffer->draw_indexed(_size, _i_count);
}

void command_buffer::end() const {
    base_command_buffer->end();
}