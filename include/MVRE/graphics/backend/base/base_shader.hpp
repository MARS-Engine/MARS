#ifndef __MVRE__BASE__SHADER__
#define __MVRE__BASE__SHADER__

#include "MVRE/resources/resource_manager.hpp"
#include "base_types.hpp"

#include <string>
#include <map>

namespace mvre_graphics_base {

    class base_shader : public mvre_resources::resource_base {
    protected:
        static std::map<std::string, MVRE_SHADER_TOKEN> module_tokens;
        static std::map<std::string, MVRE_UNIFORM_TYPE> uniform_tokens;

        std::map<MVRE_SHADER_TYPE, std::string> m_modules;
        std::vector<mvre_shader_uniform*> m_uniforms;

        bool load_shader_file(std::string _path);
    public:

        bool load_resource(const std::string& _path) override;
        void clean() override;

        virtual void bind() {  }

        mvre_shader_uniform* get_uniform(std::string _name);
    };
}

#endif