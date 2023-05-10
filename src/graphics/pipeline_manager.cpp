#include <MARS/graphics/pipeline_manager.hpp>
#include <MARS/graphics/backend/template/framebuffer.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

using namespace mars_graphics;

pl::safe_map<std::type_index, std::shared_ptr<mars_shader_inputs>> pipeline_manager::m_input_map;
pl::safe_map<std::pair<std::shared_ptr<mars_shader_inputs>, mars_ref<shader>>, std::shared_ptr<pipeline>> pipeline_manager::m_pipelines;

mars_ref<pipeline> pipeline_manager::load_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const std::shared_ptr<render_pass>& _render_pass) {
    auto id = std::make_pair(_input, _shader);
    m_pipelines.lock();
    auto result = m_pipelines[id];
    m_pipelines.unlock();

    if (result != nullptr)
        return mars_ref<pipeline>(result);
    result = prepare_pipeline(_input, _shader, _graphics, _render_pass).build();
    m_pipelines.lock();
    m_pipelines[id] = result;
    m_pipelines.unlock();
    return mars_ref<pipeline>(result);
}

pipeline_builder pipeline_manager::prepare_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const std::shared_ptr<render_pass>& _render_pass) {
    auto builder = _graphics->builder<pipeline_builder>();
    builder.set_shader_input(*_input);
    builder.set_shader(_shader);
    builder.set_render_pass(_render_pass == nullptr ? _graphics->backend().lock()->get_renderer()->get_framebuffer("main_render")->get_render_pass() : _render_pass);
    return builder;
}

void pipeline_manager::destroy() {
    m_pipelines.clear();
    m_input_map.clear();
}