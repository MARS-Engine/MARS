#ifndef __MVRE__GL__SHADER__
#define __MVRE__GL__SHADER__

#include "MVRE/graphics/backend/base/base_shader.hpp"

#include <glad/glad.h>

namespace mvre_graphics_opengl {

    struct gl_shader_uniform : mvre_graphics_base::mvre_shader_uniform {
        int id;

        explicit gl_shader_uniform(mvre_graphics_base::mvre_shader_uniform& uni) : mvre_graphics_base::mvre_shader_uniform(uni) { id = 0; }

        void update(void *_data) override;
    };

    class gl_shader : public mvre_graphics_base::base_shader {
    private:
        unsigned int id;

        std::map<mvre_graphics_base::MVRE_SHADER_TYPE, unsigned int> m_shaders_modules;

        void generate_shader(mvre_graphics_base::MVRE_SHADER_TYPE _type, const char* _data);
    public:
        bool load_resource(const std::string &_path) override;
        inline void bind() override { glUseProgram(id); }
    };
}

#endif