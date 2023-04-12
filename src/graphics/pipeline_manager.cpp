#include <MARS/graphics/pipeline_manager.hpp>
#include <MARS/graphics/backend/template/framebuffer.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

using namespace mars_graphics;

pl::safe_map<std::type_index, std::shared_ptr<mars_shader_inputs>> pipeline_manager::m_input_map;
pl::safe_map<std::pair<std::shared_ptr<mars_shader_inputs>, mars_ref<shader>>, std::shared_ptr<pipeline>> pipeline_manager::m_pipelines;

mars_ref<pipeline> pipeline_manager::load_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const mars_ref<render_pass>& _render_pass) {
    auto id = std::make_pair(_input, _shader);
    m_pipelines.lock();
    auto result = m_pipelines[id];
    m_pipelines.unlock();

    if (result != nullptr)
        return mars_ref<pipeline>(result);
    result = prepare_pipeline(_input, _shader, _graphics, _render_pass).get().lock();
    result->create();
    return mars_ref<pipeline>(result);
}

mars_ref<pipeline> pipeline_manager::prepare_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const mars_ref<render_pass>& _render_pass) {
    auto id = std::make_pair(_input, _shader);
    m_pipelines.lock();
    auto result = m_pipelines[id];

    if (result != nullptr) {
        m_pipelines.unlock();
        return mars_ref<pipeline>(result);
    }

    result = _graphics->create<pipeline>().get().lock();
    result->set_shader_input(*_input);
    result->set_shader(_shader);
    result->set_render_pass(!_render_pass.is_alive() ? _graphics->backend()->get_renderer()->get_framebuffer("main_render")->get_render_pass() : _render_pass);

    m_pipelines[id] = result;
    m_pipelines.unlock();
    return mars_ref<pipeline>(result);
}

void pipeline_manager::destroy() {
    for (auto& pipe : m_pipelines)
        pipe.second->destroy();
    m_pipelines.clear();
    m_input_map.clear();
}