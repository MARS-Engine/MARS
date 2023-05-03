#ifndef MARS_SHADER_DATA_BUILDER_
#define MARS_SHADER_DATA_BUILDER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "../graphics_builder.hpp"
#include <map>
#include <MARS/memory/mars_ref.hpp>

namespace mars_graphics {
    class texture;
    class shader;
    class pipeline;
    class shader_data;

    struct shader_data_core {
        std::map<std::string, std::shared_ptr<texture>> m_textures;
        mars_ref<shader> m_shader;
        mars_ref<pipeline> m_pipeline;
    };

    class shader_data_builder : graphics_builder<shader_data> {
    private:
        shader_data_core m_data;
    public:
        shader_data_builder() = delete;

        using graphics_builder::graphics_builder;

        inline shader_data_builder& set_textures(const std::map<std::string, std::shared_ptr<texture>>& _textures) {
            m_data.m_textures = _textures;
            return *this;
        }

        std::shared_ptr<shader_data> build(const mars_ref<pipeline>& _pipeline, const mars_ref<shader>& _shader);
    };
}

#endif