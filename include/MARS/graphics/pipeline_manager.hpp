#ifndef MARS_PIPELINE_MANAGER_
#define MARS_PIPELINE_MANAGER_

#include <typeindex>
#include <pl/safe_map.hpp>
#include "backend/template/pipeline.hpp"
#include "graphics_engine.hpp"

namespace mars_graphics {

    class pipeline_manager {
    private:
        static pl::safe_map<std::type_index, std::shared_ptr<mars_shader_inputs>> m_input_map;
        static pl::safe_map<std::pair<std::shared_ptr<mars_shader_inputs>, std::shared_ptr<shader>>, std::shared_ptr<pipeline>> m_pipelines;
    public:
        template<typename T> static inline void add_input(std::shared_ptr<mars_shader_inputs> _input) { m_input_map[typeid(T)] = _input; }
        template<typename T> static inline std::shared_ptr<mars_shader_inputs> get_input() {
            static_assert(std::is_function_v<decltype(T::get_description)>, "MARS - Pipeline Manager - Type Missing static get_description");
            auto type_index = std::type_index(typeid(T));

            if (!m_input_map.contains(type_index)) {
                auto result = T::get_description();
                m_input_map[type_index] = result;
                return result;
            }

            return m_input_map[type_index];
        }

        static std::shared_ptr<pipeline> load_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const std::shared_ptr<shader>& _shader, const mars_graphics::graphics_engine& _graphics, const std::shared_ptr<render_pass>& _render_pass = nullptr);
        static std::shared_ptr<pipeline> prepare_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const std::shared_ptr<shader>& _shader, const mars_graphics::graphics_engine& _graphics, const std::shared_ptr<render_pass>& _render_pass = nullptr);

        static void destroy();
    };
}

#endif