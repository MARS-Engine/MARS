#ifndef MARS_GL_SHADER_INPUT_
#define MARS_GL_SHADER_INPUT_

#include <MARS/graphics/backend/template/shader_input.hpp>
#include <glad/glad.h>

namespace mars_graphics {

    class gl_shader_input : public shader_input {
    private:
        unsigned int m_id;
    public:
        using shader_input::shader_input;

        void create() override;

        buffer* add_buffer(size_t _input_size, MARS_MEMORY_TYPE _input_type) override;

        void load_input(mars_shader_inputs _inputs) override;

        inline void bind() override { glBindVertexArray(m_id); }
        inline void unbind() override { glBindVertexArray(0); }
    };
}

#endif