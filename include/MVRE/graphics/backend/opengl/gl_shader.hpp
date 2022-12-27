#ifndef MVRE_GL_SHADER_
#define MVRE_GL_SHADER_

#include <MVRE/graphics/backend/template/shader.hpp>
#include <glad/glad.h>

namespace mvre_graphics {

    class gl_shader : public shader {
    private:
        unsigned int id;

        std::map<MVRE_SHADER_TYPE, unsigned int> m_shaders_modules;

        void generate_shader(MVRE_SHADER_TYPE _type, const std::string& _data) override;
    public:
        using shader::shader;

        inline unsigned int gl_id() { return id; }

        bool load_resource(const std::string &_path) override;
        inline void bind() override { glUseProgram(id); }
    };
}

#endif