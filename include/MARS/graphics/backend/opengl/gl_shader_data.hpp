#ifndef MARS_GL_SHADER_DATA_
#define MARS_GL_SHADER_DATA_

#include "gl_buffer.hpp"
#include "gl_shader.hpp"
#include <MARS/graphics/backend/template/shader_data.hpp>
#include <glad/glad.h>

namespace mars_graphics {

    class gl_uniform : uniform {
    public:
        using uniform::uniform;

        void bind(size_t _index) override {
            glBindBufferBase(GL_UNIFORM_BUFFER, m_index, ((gl_buffer*)m_buffer)->id());
        }
    };

    class gl_shader_data : public shader_data {
    public:
        using shader_data::shader_data;

        void bind(size_t _frame) override {
            for (auto& uni : m_uniforms)
                uni.second->bind(_frame);
            for (auto& tex : m_textures)
                tex.second->bind();
        }

        void bind_textures() override {
            for (auto& tex : m_textures)
                tex.second->bind();
        }

        void generate(pipeline* _pipeline, shader* _shader) override;
    };
}

#endif