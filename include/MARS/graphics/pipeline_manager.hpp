#ifndef MARS_PIPELINE_MANAGER_
#define MARS_PIPELINE_MANAGER_

#include <typeindex>
#include <pl/safe.hpp>
#include <MARS/engine/singleton.hpp>
#include "backend/template/pipeline.hpp"
#include "graphics_engine.hpp"

namespace mars_graphics {

    class pipeline_manager : public mars_engine::singleton {
    private:
        pl::safe<std::map<std::pair<mars_shader_inputs, mars_ref<shader>>, std::shared_ptr<pipeline>>> m_pipelines;
    public:
        ~pipeline_manager();

        mars_ref<pipeline> load_pipeline(const mars_shader_inputs& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const std::shared_ptr<render_pass>& _render_pass = nullptr);
        pipeline_builder prepare_pipeline(const mars_shader_inputs& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const std::shared_ptr<render_pass>& _render_pass = nullptr);
    };
}

#endif