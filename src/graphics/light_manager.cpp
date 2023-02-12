#include <MARS/graphics/light_manager.hpp>
#include <MARS/graphics/backend/template/command_buffer.hpp>
#include <MARS/graphics/renderer/renderer.hpp>
#include <MARS/graphics/attribute/vertex2.hpp>
#include <glad/glad.h>

using namespace mars_graphics;


float quadVertices[] = {
        // positions        // texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};


void light_manager::load(graphics_instance* _instance) {
    m_instance = _instance;

    //NOTE: temp
    if (m_instance->render_type() != "deferred")
        return;

    if (!mars_resources::resource_manager::load_graphical_resource(mars_resources::resource_manager::find_path("light.mshader", MARS_RESOURCE_TYPE_SHADER, m_instance->render_type()),light_shader, m_instance))
        mars_debug::debug::error("MARS - OpenGL - Backend - Failed to find light shader");

    m_pipeline = pipeline_manager::prepare_pipeline(pipeline_manager::get_input<vertex2>(), light_shader, m_instance, m_instance->backend()->get_renderer()->get_framebuffer("light_render")->get_render_pass());
    m_pipeline->set_viewport({ 0, 0 }, {1920, 1080 }, {0, 1 });
    m_pipeline->set_flip_y(false);
    m_pipeline->set_topology(MARS_TOPOLOGY_TRIANGLE_STRIP);
    m_pipeline->create();

    std::map<std::string, texture*> input_textures {
            {"gPosition", m_instance->backend()->get_renderer()->get_framebuffer("main_render")->get_texture(0)},
            {"gNormal", m_instance->backend()->get_renderer()->get_framebuffer("main_render")->get_texture(1)},
            {"gAlbedoSpec", m_instance->backend()->get_renderer()->get_framebuffer("main_render")->get_texture(2)}
    };

    m_data = m_instance->instance<shader_data>();
    m_data->set_textures(input_textures);
    m_data->generate(m_pipeline, light_shader);

    m_input = m_instance->instance<shader_input>();
    m_input->create();
    m_input->bind();

    auto vertex = m_input->add_buffer(sizeof(quadVertices), MARS_MEMORY_TYPE_VERTEX);
    vertex->copy_data(&quadVertices);

    m_input->load_input(vertex2::get_description());

    m_input->unbind();
}

void light_manager::draw_lights() {
    if (m_instance->backend()->get_renderer()->get_render_type() == "forward")
        return;

    m_instance->backend()->get_renderer()->get_framebuffer("light_render")->get_render_pass()->begin();
    m_pipeline->bind();
    light_shader->bind();
    update_buffer.lock();
    lights.insert(lights.end(), update_buffer.begin(), update_buffer.end());
    update_buffer.clear();
    update_buffer.unlock();

    mars_math::vector4<float> pos = { m_instance->get_camera().position().x(), m_instance->get_camera().position().y(), m_instance->get_camera().position().z(), 0 };

    auto camera_uni =  m_data->get_uniform("camera");

    camera_uni->bind(m_instance->current_frame());
    camera_uni->update(&pos);
    camera_uni->copy_data(m_instance->backend()->current_frame());
    auto light_uniform = m_data->get_uniform("lights");
    light_uniform->bind(m_instance->current_frame());
    light_uniform->update(&scene);
    m_data->bind();
    m_input->bind();

    //dwqd
    int active_lights = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        scene.lights[active_lights++] = *lights[i];
        scene.active_lights = active_lights;

        if (active_lights == 32 || lights.size() == i + 1) {
            active_lights = 0;
            light_uniform->copy_data(m_instance->backend()->current_frame());
            m_instance->primary_buffer()->draw(0, 4);
        }
    }
    m_instance->backend()->get_renderer()->get_framebuffer("light_render")->get_render_pass()->end();
}

void light_manager::destroy() {
    if (m_instance->backend()->get_renderer()->get_render_type() == "forward")
        return;

    m_data->destroy();
    delete m_data;

    m_input->destroy();
    delete m_input;
}