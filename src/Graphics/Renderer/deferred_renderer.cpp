#include "deferred_renderer.hpp"
#include "../window.hpp"
#include "../Vulkan/vtexture.hpp"
#include "../Vulkan/vdevice.hpp"
#include "../Vulkan/vpipeline.hpp"
#include "Manager/light_manager.hpp"
#include "../Vulkan/vdepth.hpp"

void deferred_renderer::create_texture(const string& _name, deferred_texture_type _type) {
    auto new_texture = new texture(engine);

    VkFormat format;

    switch (_type) {
        case COLOR:
            format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case POSITION:
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
    }

    new_texture->create(engine->surf_window->size, format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    textures.push_back({_name, _type, new_texture });
}

vrender_pass *deferred_renderer::get_render_pass() {
    auto render_pass = render_pass_manager::get_render_pass("DeferredRenderer", engine, {.should_load = false});
    render_pass->prepare(textures_data);
    render_pass->add_depth(engine->surf_window->size);
    render_pass->create();
    return render_pass;
}

VkFramebuffer deferred_renderer::get_framebuffer(size_t _index) {
    return framebuffer->raw_framebuffers[0];
}

void deferred_renderer::clear() {
    engine->draw_queue.push_back(clear_buffer->get_command_buffer());
}

void deferred_renderer::load() {
    auto deferred_render = render_pass_manager::get_render_pass("Renderer", engine);
    transform(textures.begin(), textures.end(), back_inserter(textures_data), [](auto& t) { return t._texture; });
    deferred_render->prepare(textures_data);
    deferred_render->add_depth(textures[0]._texture->get_size());
    deferred_render->create();

    auto shader = shader_manager::get_shader("Deferred.shader", engine);

    auto render_pipeline = pipeline_manager::get_pipeline("Renderer");

    if (render_pipeline == nullptr) {
        render_pipeline = new pipeline(engine, shader, render_pass_manager::get_render_pass("default", engine));
        render_pipeline->create_layout();
        render_pipeline->apply_viewport({.size = engine->surf_window->size, .flip_y = false});
        render_pipeline->create();
        pipeline_manager::add_pipeline("Renderer", render_pipeline);
    }

    data = new shader_data(shader, engine);

    for (auto& t : textures)
        data->get_uniform(t.name)->setTexture(t._texture);

    light_manager::generate_shader_uniform(data);

    data->generate();

    render_buffer = new command_buffer(engine);
    render_buffer->render_pass = render_pass_manager::get_render_pass("default", engine);
    render_buffer->create();

    for (int i = 0; i < vengine::frame_overlap; i++) {
        render_buffer->begin(i);
        render_buffer->load_default(i, engine->framebuffer);
        render_pipeline->bind(render_buffer);
        data->bind(render_buffer, render_pipeline);
        render_buffer->draw(3, 1);
        render_buffer->end();
    }

    framebuffer = new vframebuffer();
    framebuffer->set_attachments(textures_data);
    framebuffer->add_attachment(render_buffer->render_pass->depth->image_view);
    framebuffer->create(deferred_render, textures_data[0]->get_size());

    clear_buffer = new command_buffer(engine);
    clear_buffer->render_pass = get_render_pass();
    clear_buffer->create();

    for (int i = 0; i < vengine::frame_overlap; i++) {
        clear_buffer->begin(i);
        clear_buffer->load_default(i);
        clear_buffer->end();
    }
}

void deferred_renderer::update() {
    light_manager::update_shader_uniform(data);
}

void deferred_renderer::render() {
    engine->draw_queue.push_back(render_buffer->get_command_buffer());
}