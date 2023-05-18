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
        static pl::safe_map<std::pair<std::shared_ptr<mars_shader_inputs>, mars_ref<shader>>, std::shared_ptr<pipeline>> m_pipelines;
    public:
        template<typename T> static inline void add_input(const std::shared_ptr<mars_shader_inputs>& _input) { m_input_map.lock()->insert(std::make_pair(std::type_index(typeid(T)), _input)); }
        template<typename T> static inline std::shared_ptr<mars_shader_inputs> get_input() {
            static_assert(std::is_function_v<decltype(T::get_description)>, "MARS - Pipeline Manager - Type Missing static get_description");
            auto type_index = std::type_index(typeid(T));

            auto input_map = m_input_map.lock();

            if (!input_map->contains(type_index)) {
                auto result = T::get_description();
                input_map->insert(std::make_pair(type_index, result));
                return result;
            }

            return input_map->at(type_index);
        }

        static mars_ref<pipeline> load_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const std::shared_ptr<render_pass>& _render_pass = nullptr);
        static pipeline_builder prepare_pipeline(const std::shared_ptr<mars_shader_inputs>& _input, const mars_ref<shader>& _shader, const mars_ref<mars_graphics::graphics_engine>& _graphics, const std::shared_ptr<render_pass>& _render_pass = nullptr);

        static void destroy();
    };
}

#endif