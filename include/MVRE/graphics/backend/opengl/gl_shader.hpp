#ifndef __MVRE__GL__SHADER__
#define __MVRE__GL__SHADER__

#include <MVRE/graphics/backend/template/shader.hpp>
#include <glad/glad.h>

namespace mvre_graphics {

    struct gl_shader_uniform : public mvre_shader_uniform {
        int id;

        explicit gl_shader_uniform(mvre_shader_uniform& uni) : mvre_shader_uniform(uni) { id = 0; }

        void update(void *_data) override;
    };

    class gl_shader : public shader {
    private:
        unsigned int id;

        std::map<MVRE_SHADER_TYPE, unsigned int> m_shaders_modules;

        void generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) override;
    public:
        using shader::shader;

        bool load_resource(const std::string &_path) override;
        inline void bind() override { glUseProgram(id); }
    };
}

#endif