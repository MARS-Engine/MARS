#ifndef MVRE_SHADER_
#define MVRE_SHADER_

#include <map>
#include <vector>
#include "graphics_types.hpp"
#include <MVRE/resources/resource_manager.hpp>
#include "graphics_component.hpp"

namespace mvre_graphics {

    class shader : public mvre_resources::resource_base, public graphics_component {
    protected:
        static std::map<std::string, MVRE_SHADER_TOKEN> module_tokens;
        static std::map<std::string, MVRE_UNIFORM_TYPE> uniform_tokens;

        std::map<MVRE_SHADER_TYPE, std::string> m_modules;
        std::vector<mvre_shader_uniform*> m_uniforms;

        virtual void generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) { }

        bool load_shader_file(std::string _path, std::string _path_sufix = "");
    public:
        using graphics_component::graphics_component;

        std::vector<mvre_shader_uniform*>& get_uniforms() { return m_uniforms; }

        bool load_resource(const std::string& _path) override;
        void clean() override;

        virtual void bind() {  }

        mvre_shader_uniform* get_uniform(const std::string& _name);
    };
}

#endif