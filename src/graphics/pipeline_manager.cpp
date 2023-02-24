#include <MARS/graphics/pipeline_manager.hpp>
#include <MARS/graphics/backend/template/framebuffer.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

using namespace mars_graphics;

pl::safe_map<std::type_index, mars_shader_inputs*> pipeline_manager::m_input_map;
pl::safe_map<std::pair<mars_shader_inputs*, shader*>, pipeline*> pipeline_manager::m_pipelines;

pipeline* pipeline_manager::load_pipeline(mars_shader_inputs* _input, shader* _shader, graphics_engine* _graphics, render_pass* _render_pass) {
    auto id = std::make_pair(_input, _shader);
    m_pipelines.lock();
    auto result = m_pipelines[id];
    m_pipelines.unlock();

    if (result != nullptr)
        return result;
    result = prepare_pipeline(_input, _shader, _graphics, _render_pass);
    result->create();
    return result;
}

pipeline* pipeline_manager::prepare_pipeline(mars_shader_inputs *_input, shader *_shader, graphics_engine *_graphics, render_pass *_render_pass) {
    auto id = std::make_pair(_input, _shader);
    m_pipelines.lock();
    auto result = m_pipelines[id];

    if (result != nullptr) {
        m_pipelines.unlock();
        return result;
    }

    result = _graphics->create<pipeline>();
    result->set_shader_input(*_input);
    result->set_shader(_shader);
    result->set_render_pass(_render_pass == nullptr ? _graphics->backend()->get_renderer()->get_framebuffer("main_render")->get_render_pass() : _render_pass);

    m_pipelines[id] = result;
    m_pipelines.unlock();
    return result;
}

void pipeline_manager::destroy() {
    for (auto& pipe : m_pipelines) {
        pipe.second->destroy();
        delete pipe.second;
    }
}