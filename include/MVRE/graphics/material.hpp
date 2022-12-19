#ifndef __MVRE__MATERIAL__
#define __MVRE__MATERIAL__

#include <MVRE/resources/resource_manager.hpp>
#include <MVRE/graphics/backend/template/backend_instance.hpp>
#include <MVRE/graphics/backend/template/graphics_types.hpp>
#include <MVRE/graphics/backend/template/shader.hpp>
#include <MVRE/graphics/backend/template/texture.hpp>
#include <MVRE/graphics/backend/template/pipeline.hpp>
#include <MVRE/graphics/backend/template/shader_data.hpp>
#include "pipeline_manager.hpp"

namespace mvre_graphics {

    class material : public mvre_resources::resource_base {
    private:
        shader* m_shader = nullptr;
        pipeline* m_pipeline = nullptr;
        std::map<std::string, texture*> m_textures;

        graphics_instance* m_instance;

        static std::map<std::string, MVRE_MATERIAL_INPUT> mat_input_tokens;
    public:
        material(graphics_instance* _instance) { m_instance = _instance; }

        inline pipeline* get_pipeline() { return m_pipeline; }

        inline shader_data* generate_shader_data() {
            auto data = m_instance->instance<shader_data>();
            data->set_textures(m_textures);
            data->generate(m_pipeline, m_shader);

            return data;
        }

        template<typename T> inline void set_pipeline() {
            m_pipeline = pipeline_manager::load_pipeline(pipeline_manager::get_input<T>(), m_shader, m_instance);
        }

        bool load_resource(const std::string &_path) override;
        void clean() override;
    };
}

#endif