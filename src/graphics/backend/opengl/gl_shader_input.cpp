#include <MVRE/graphics/backend/opengl/gl_shader_input.hpp>
#include <MVRE/graphics/backend/opengl/gl_buffer.hpp>

using namespace mvre_graphics;

void gl_shader_input::create() {
    glGenVertexArrays(1, &m_id);
}

buffer* gl_shader_input::add_buffer(size_t _input_size, MVRE_MEMORY_TYPE _input_type) {
    auto new_buffer = new gl_buffer(instance());
    new_buffer->create(_input_size, _input_type);
    new_buffer->bind();
    m_buffers.push_back(new_buffer);
    return new_buffer;
}

void gl_shader_input::load_input(mvre_shader_inputs _inputs) {
    int stride = 0;
    for (auto i = 0; i < _inputs.length; i++)
        stride += _inputs.input_data[i].stride;

    size_t accumulated_stride = 0;
    for (auto i = 0; i < _inputs.length; i++) {
        GLenum type;
        switch (_inputs.input_data[i].type) {
            case MVRE_SHADER_INPUT_TYPE_SF_RG:
            case MVRE_SHADER_INPUT_TYPE_SF_RGB:
            case MVRE_SHADER_INPUT_TYPE_SF_RGBA:
                type = GL_FLOAT;
                break;
        }

        glVertexAttribPointer(i, _inputs.input_data[i].stride, type, GL_FALSE, stride * sizeof(float), (void*)(accumulated_stride * sizeof(float)));
        glEnableVertexAttribArray(i);
        accumulated_stride += _inputs.input_data[i].stride;
    }
}