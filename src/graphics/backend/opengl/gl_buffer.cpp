#include <MVRE/graphics/backend/opengl/gl_buffer.hpp>

using namespace mvre_graphics;

void gl_buffer::create(size_t _size, MVRE_MEMORY_TYPE _mem_type) {
    buffer::create(_size, _mem_type);
    glGenBuffers(1, &m_buffer_id);

    switch (_mem_type) {
        case MVRE_MEMORY_TYPE_VERTEX:
            m_gl_type = GL_ARRAY_BUFFER;
            break;
        case MVRE_MEMORY_TYPE_INDEX:
            m_gl_type = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case MVRE_MEMORY_TYPE_UNIFORM:
            m_gl_type = GL_UNIFORM_BUFFER;
            bind();
            glBufferData(m_gl_type, static_cast<GLsizeiptr>(m_size), nullptr, GL_DYNAMIC_DRAW);
            unbind();
            glBindBufferRange(m_gl_type, 0, m_buffer_id, 0, static_cast<GLsizeiptr>(m_size));
            break;
    }
}