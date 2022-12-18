#ifndef __MVRE__GL__SHADER__DATA__
#define __MVRE__GL__SHADER__DATA__

#include "gl_buffer.hpp"
#include "gl_shader.hpp"
#include <MVRE/graphics/backend/template/shader_data.hpp>
#include <glad/glad.h>

namespace mvre_graphics {

    class gl_uniform : uniform {
    public:
        using uniform::uniform;

        void bind() override {
            glBindBufferBase(GL_UNIFORM_BUFFER, m_index, ((gl_buffer*)m_buffer)->id());
        }
    };

    class gl_shader_data : public shader_data {
    public:
        using shader_data::shader_data;

        void bind() override {
            for (auto& uni : m_uniforms)
                uni.second->bind();
        }

        void generate(shader* _shader) override;
    };
}

#endif