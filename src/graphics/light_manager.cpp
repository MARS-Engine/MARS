#include <MARS/graphics/light_manager.hpp>
#include <MARS/graphics/backend/template/command_buffer.hpp>
#include <MARS/graphics/renderer/renderer.hpp>
#include <MARS/graphics/attribute/vertex2.hpp>

using namespace mars_graphics;


float quadVertices[] = {
        // positions        // texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};


void light_manager::load(const mars_ref<mars_graphics::graphics_engine>& _graphics) {
    m_graphics = _graphics;

    //NOTE: temp
    if (m_graphics->render_type() != "deferred")
        return;

    light_shader = m_graphics->builder<shader_builder>().load_from_file(m_graphics->resources()->find_path("light.mshader", MARS_RESOURCE_TYPE_SHADER, m_graphics->render_type())).build();
    //if (!m_graphics->resources()->load_graphical_resource(, light_shader, m_graphics))
    //   mars_debug::debug::error("MARS - Vulkan - Backend - Failed to find light shader");

    auto pipe_builder = pipeline_manager::prepare_pipeline(pipeline_manager::get_input<vertex2>(), mars_ref<shader>(light_shader), m_graphics, m_graphics->backend().lock()->get_renderer()->get_framebuffer("light_render")->get_render_pass());
    pipe_builder.set_viewport({ 0, 0 }, {1920, 1080 }, {0, 1 });
    pipe_builder.set_flip_y(false);
    pipe_builder.set_topology(MARS_TOPOLOGY_TRIANGLE_STRIP);
    m_pipeline = pipe_builder.build();

    std::map<std::string, std::shared_ptr<texture>> input_textures {
            {"gPosition",   m_graphics->backend().lock()->get_renderer()->get_framebuffer("main_render")->get_texture(0)},
            {"gNormal",     m_graphics->backend().lock()->get_renderer()->get_framebuffer("main_render")->get_texture(1)},
            {"gAlbedoSpec", m_graphics->backend().lock()->get_renderer()->get_framebuffer("main_render")->get_texture(2)}
    };

    m_data = m_graphics->builder<shader_data_builder>().set_textures(input_textures).build(mars_ref<pipeline>(m_pipeline), mars_ref<shader>(light_shader));

    auto builder = m_graphics->builder<shader_input_builder>();

    auto vertex = builder.add_buffer(sizeof(quadVertices), MARS_MEMORY_TYPE_VERTEX);
    vertex->update(&quadVertices);
    vertex->copy_data(0);

    builder.load_input(vertex2::get_description());

    m_input = builder.build();
}

void light_manager::draw_lights() {
    if (m_graphics->backend().lock()->get_renderer()->get_render_type() == "forward")
        return;

    m_graphics->backend().lock()->get_renderer()->get_framebuffer("light_render")->get_render_pass()->begin();
    m_pipeline->bind();
    light_shader->bind();

    auto buffer_vec = update_buffer.lock();
    auto lights_vec = lights.lock();

    lights_vec->insert(lights_vec->end(), buffer_vec->begin(), buffer_vec->end());

    buffer_vec->clear();

    mars_math::vector4<float> pos = {m_graphics->get_camera().position().x, m_graphics->get_camera().position().y, m_graphics->get_camera().position().z, 0 };

    auto camera_uni =  m_data->get_uniform("camera");

    camera_uni->bind(m_graphics->current_frame());
    camera_uni->update(&pos);
    camera_uni->copy_data(m_graphics->backend().lock()->current_frame());
    auto light_uniform = m_data->get_uniform("lights");
    light_uniform->bind(m_graphics->current_frame());
    m_data->bind();
    m_input->bind();

    //dwqd
    int active_lights = 0;
    for (size_t i = 0; i < lights_vec->size(); i++) {
        scene.lights[active_lights++] = *lights_vec->at(i);
        scene.active_lights = active_lights;

        if (active_lights == 32 || lights_vec->size() == i + 1) {
            active_lights = 0;
            light_uniform->update(&scene);
            light_uniform->copy_data(m_graphics->backend().lock()->current_frame());
            m_graphics->primary_buffer()->draw(0, 4);
        }
    }
    m_graphics->backend().lock()->get_renderer()->get_framebuffer("light_render")->get_render_pass()->end();
}

void light_manager::destroy() {
    if (m_graphics->backend().lock()->get_renderer()->get_render_type() == "forward")
        return;

    m_data.reset();
    m_input.reset();
}