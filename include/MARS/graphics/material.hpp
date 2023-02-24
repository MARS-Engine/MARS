#ifndef MARS_MATERIAL_
#define MARS_MATERIAL_

#include <MARS/resources/resource_manager.hpp>
#include <MARS/graphics/backend/template/graphics_backend.hpp>
#include <MARS/graphics/backend/template/graphics_types.hpp>
#include <MARS/graphics/backend/template/shader.hpp>
#include <MARS/graphics/backend/template/texture.hpp>
#include <MARS/graphics/backend/template/pipeline.hpp>
#include <MARS/graphics/backend/template/shader_data.hpp>
#include "pipeline_manager.hpp"

namespace mars_graphics {

    class material : public mars_resources::resource_base {
    private:
        shader* m_shader = nullptr;
        pipeline* m_pipeline = nullptr;
        std::map<std::string, texture*> m_textures;

        graphics_engine* m_graphics;

        static std::map<std::string, MARS_MATERIAL_INPUT> mat_input_tokens;
    public:
        material(graphics_engine* _instance) { m_graphics = _instance; }

        inline pipeline* get_pipeline() { return m_pipeline; }

        inline void bind() {
            m_shader->bind();
        }

        inline shader_data* generate_shader_data() {
            auto data = m_graphics->create<shader_data>();
            data->set_textures(m_textures);
            data->generate(m_pipeline, m_shader);

            return data;
        }

        template<typename T> inline void set_pipeline() {
            m_pipeline = pipeline_manager::load_pipeline(pipeline_manager::get_input<T>(), m_shader, m_graphics);
        }

        bool load_resource(const std::string &_path) override;
        void clean() override;
    };
}

#endif