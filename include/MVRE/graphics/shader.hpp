#ifndef __MVRE__SHADER__
#define __MVRE__SHADER__

#include "backend/base/base_shader.hpp"

namespace mvre_graphics {
    
    class shader : public mvre_resources::resource_base {
        private:
            mvre_graphics_base::base_shader* raw_shader;
        public:
            
            bool load_resource(const std::string& _path) override;
            inline mvre_graphics_base::mvre_shader_uniform* get_uniform(const std::string& _name) { return raw_shader->get_uniform(_name); }
            inline void bind() { raw_shader->bind(); }
            inline void clean() override { raw_shader->clean(); }
    };
};

#endif