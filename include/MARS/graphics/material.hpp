#ifndef MARS_MATERIAL_
#define MARS_MATERIAL_

#include <MARS/resources/resource_manager.hpp>
#include <MARS/graphics/backend/template/graphics_backend.hpp>
#include <MARS/graphics/backend/template/graphics_types.hpp>
#include <MARS/graphics/backend/template/shader.hpp>
#include <MARS/graphics/backend/template/texture.hpp>
#include <MARS/graphics/backend/template/pipeline.hpp>
#include <MARS/graphics/backend/template/shader_data.hpp>
#include <MARS/engine/object_engine.hpp>
#include "pipeline_manager.hpp"

namespace mars_graphics {

    class material : public mars_resources::resource_base {
    private:
        std::shared_ptr<shader> m_shader;
        mars_ref<pipeline> m_pipeline;
        std::map<std::string, std::shared_ptr<texture>> m_textures;

        mars_ref<mars_graphics::graphics_engine> m_graphics;

        static std::map<std::string, MARS_MATERIAL_INPUT> mat_input_tokens;
    public:
        explicit material(const mars_ref<mars_graphics::graphics_engine>& _instance) { m_graphics = _instance; }

        inline mars_ref<pipeline> get_pipeline() { return m_pipeline; }

        inline void bind() {
            m_shader->bind();
        }

        [[nodiscard]] inline std::shared_ptr<shader_data> generate_shader_data() const {
            return m_graphics->builder<shader_data_builder>().set_textures(m_textures).build(m_pipeline, mars_ref<shader>(m_shader));
        }

        template<typename T> requires std::is_function_v<decltype(T::description)> inline void set_pipeline() {
            auto val = m_graphics->engine()->get<pipeline_manager>()->load_pipeline(T::description(), mars_ref<shader>(m_shader), m_graphics);
            m_pipeline = val;
        }

        bool load_resource(const std::string &_path) override;
        void clean() override;
    };
}

#endif