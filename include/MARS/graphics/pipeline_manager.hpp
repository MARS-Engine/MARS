#ifndef MARS_PIPELINE_MANAGER_
#define MARS_PIPELINE_MANAGER_

#include <pl/safe_map.hpp>
#include "backend/template/pipeline.hpp"
#include "graphics_instance.hpp"

namespace mars_graphics {

    class pipeline_manager {
    private:
        static pl::safe_map<size_t, mars_shader_inputs*> m_input_map;
        static pl::safe_map<std::pair<mars_shader_inputs*, shader*>, pipeline*> m_pipelines;
    public:
        template<typename T> static inline void add_input(mars_shader_inputs* _input) { m_input_map[typeid(T).hash_code()] = _input; }
        template<typename T> static inline mars_shader_inputs* get_input() {
            static_assert(std::is_function_v<decltype(T::get_description)>, "MARS - Pipeline Manager - Type Missing static get_description");
            auto hash = typeid(T).hash_code();
            auto result = m_input_map[hash];

            if (result == nullptr) {
                result = &T::get_description();
                m_input_map[hash] = result;
            }

            return result;
        }

        static pipeline* load_pipeline(mars_shader_inputs* _input, shader* _shader, graphics_instance* _graphics_instance);

        static void destroy();
    };
}

#endif