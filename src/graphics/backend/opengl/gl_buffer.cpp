#include <MARS/graphics/backend/opengl/gl_buffer.hpp>

using namespace mars_graphics;

unsigned int gl_buffer::id() {
    return m_ids[instance()->current_frame()];
}

void gl_buffer::bind() {
    glBindBuffer(m_gl_type, m_ids[instance()->current_frame()]);
}

void gl_buffer::create(size_t _size, MARS_MEMORY_TYPE _mem_type) {
    buffer::create(_size, _mem_type);
    m_ids.resize(instance()->max_frames(), 0);
    glGenBuffers((int)instance()->max_frames(), m_ids.data());

    switch (_mem_type) {
        case MARS_MEMORY_TYPE_VERTEX:
            m_gl_type = GL_ARRAY_BUFFER;
            break;
        case MARS_MEMORY_TYPE_INDEX:
            m_gl_type = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case MARS_MEMORY_TYPE_UNIFORM:
            m_gl_type = GL_UNIFORM_BUFFER;
            for (auto& id : m_ids) {
                glBindBuffer(m_gl_type, id);
                glBufferData(m_gl_type, static_cast<GLsizeiptr>(m_size), nullptr, GL_DYNAMIC_DRAW);
                unbind();
                glBindBufferRange(m_gl_type, 0, id, 0, static_cast<GLsizeiptr>(m_size));
            }
            break;
    }
}
