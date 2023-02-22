#ifndef MARS_PIPELINE_MANAGER_
#define MARS_PIPELINE_MANAGER_

#include <typeindex>
#include <pl/safe_map.hpp>
#include "backend/template/pipeline.hpp"
#include "graphics_instance.hpp"

namespace mars_graphics {

    class pipeline_manager {
    private:
        static pl::safe_map<std::type_index, mars_shader_inputs*> m_input_map;
        static pl::safe_map<std::pair<mars_shader_inputs*, shader*>, pipeline*> m_pipelines;
    public:
        template<typename T> static inline void add_input(mars_shader_inputs* _input) { m_input_map[typeid(T)] = _input; }
        template<typename T> static inline mars_shader_inputs* get_input() {
            static_assert(std::is_function_v<decltype(T::get_description)>, "MARS - Pipeline Manager - Type Missing static get_description");
            auto type_index = std::type_index(typeid(T));

            if (!m_input_map.contains(type_index)) {
                auto result = &T::get_description();
                m_input_map[type_index] = result;
                return result;
            }

            return m_input_map[type_index];
        }

        static pipeline* load_pipeline(mars_shader_inputs* _input, shader* _shader, graphics_instance* _graphics_instance, render_pass* _render_pass = nullptr);
        static pipeline* prepare_pipeline(mars_shader_inputs* _input, shader* _shader, graphics_instance* _graphics_instance, render_pass* _render_pass = nullptr);

        static void destroy();
    };
}

#endif