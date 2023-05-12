#ifndef MARS_SHADER_
#define MARS_SHADER_

#include <map>
#include <vector>
#include "graphics_types.hpp"
#include <MARS/resources/resource_manager.hpp>
#include "graphics_component.hpp"
#include "builders/shader_builder.hpp"
#include "resources/shader_resource.hpp"

namespace mars_graphics {

    class shader : public graphics_component {
    protected:
        mars_ref<mars_graphics::shader_resource> m_data;

        inline void set_data(const mars_ref<mars_graphics::shader_resource>& _data) {
            m_data = _data;
        }

        virtual void generate_shader(MARS_SHADER_TYPE _type, const std::string& _data) { }

        virtual bool load_shader(const mars_ref<shader_resource>& _resource) { return false; }

        virtual std::string get_suffix() { return ""; }
    public:
        static const std::map<std::string, MARS_SHADER_TOKEN> module_tokens;
        static const std::map<std::string, MARS_UNIFORM_TYPE> uniform_tokens;

        friend shader_builder;

        using graphics_component::graphics_component;

        std::vector<std::shared_ptr<mars_shader_uniform>>& get_uniforms() { return m_data->data().uniforms; }

        virtual void bind() {  }

        std::shared_ptr<mars_shader_uniform> get_uniform(const std::string& _name);
    };
}

#endif