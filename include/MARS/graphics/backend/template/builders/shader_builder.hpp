#ifndef MARS_SHADER_BUILDER_
#define MARS_SHADER_BUILDER_

#include "graphics_builder.hpp"
#include <MARS/memory/mars_ref.hpp>
#include "../graphics_types.hpp"
#include <map>
#include <vector>

namespace mars_graphics {
    class shader;
    class shader_builder;
    class shader_resource;

    struct shader_core_data {
        std::map<MARS_SHADER_TYPE, std::string> modules;
        std::vector<std::shared_ptr<mars_shader_uniform>> uniforms;
    };

    class shader_builder : graphics_builder<shader> {
    private:
        shader_core_data m_data;
        mars_ref<shader_resource> m_shader_resource;
    public:
        using graphics_builder::graphics_builder;

        shader_builder& load_from_file(const std::string& _path);

        std::shared_ptr<shader> build();
    };
}

#endif