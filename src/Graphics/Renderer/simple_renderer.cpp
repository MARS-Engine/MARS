#include "simple_renderer.hpp"
#include "Manager/render_pass_manager.hpp"
#include "Graphics/window.hpp"
#include "Graphics/Vulkan/vswapchain.hpp"
#include "Graphics/Vulkan/vframebuffer.hpp"

VkFramebuffer simple_renderer::get_framebuffer(size_t _index) {
    return engine->framebuffer->raw_framebuffers[_index];
}

void simple_renderer::load() {
    auto renderer = render_pass_manager::get_render_pass("Renderer", engine);
    renderer->prepare({});
    renderer->add_description(engine->swapchain->format);
    renderer->add_depth(engine->surf_window->size);
    renderer->create();

    clear_buffer = new command_buffer(engine);
    clear_buffer->render_pass = engine->renderPass;
    clear_buffer->create();

    for (int i = 0; i < vengine::frame_overlap; i++) {
        clear_buffer->begin(i);
        clear_buffer->load_default(i);
        clear_buffer->end();
    }
}
void simple_renderer::clear() {
    engine->draw_queue.push_back(clear_buffer->get_command_buffer());
}