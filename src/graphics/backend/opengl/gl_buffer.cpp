#include "MVRE/graphics/backend/opengl/gl_buffer.hpp"

using namespace mvre_graphics_opengl;
using namespace mvre_graphics_base;

void gl_buffer::create(size_t _size, MVRE_MEMORY_TYPE _mem_type) {
    base_buffer::create(_size, _mem_type);
    glGenBuffers(1, &m_buffer_id);

    switch (_mem_type) {
        case MVRE_MEMORY_TYPE_VERTEX:
            m_gl_type = GL_ARRAY_BUFFER;
            break;
        case MVRE_MEMORY_TYPE_INDEX:
            m_gl_type = GL_ELEMENT_ARRAY_BUFFER;
            break;
    }
}