#include <MVRE/graphics/pipeline_manager.hpp>

using namespace mvre_graphics;

pl::safe_map<size_t, mvre_shader_inputs*> pipeline_manager::m_input_map;
pl::safe_map<std::pair<mvre_shader_inputs*, shader*>, pipeline*> pipeline_manager::m_pipelines;

pipeline* pipeline_manager::load_pipeline(mvre_shader_inputs* _input, shader* _shader, graphics_instance* _graphics_instance) {
    auto id = std::make_pair(_input, _shader);
    auto result = m_pipelines[id];

    if (result != nullptr)
        return result;

    result = _graphics_instance->instance<pipeline>();
    result->set_shader_input(*_input);
    result->set_shader(_shader);
    result->create();

    m_pipelines[id] = result;

    return result;
}

void pipeline_manager::destroy() {
    for (auto& pipe : m_pipelines) {
        pipe.second->destroy();
        delete pipe.second;
    }
}