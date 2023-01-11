#ifndef MARS_GL_SHADER_
#define MARS_GL_SHADER_

#include <MARS/graphics/backend/template/shader.hpp>
#include <glad/glad.h>

namespace mars_graphics {

    class gl_shader : public shader {
    private:
        unsigned int id;

        std::map<MARS_SHADER_TYPE, unsigned int> m_shaders_modules;

        void generate_shader(MARS_SHADER_TYPE _type, const std::string& _data) override;
    public:
        using shader::shader;

        inline unsigned int gl_id() { return id; }

        bool load_resource(const std::string &_path) override;
        inline void bind() override { glUseProgram(id); }
    };
}

#endif