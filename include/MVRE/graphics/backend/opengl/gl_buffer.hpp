#ifndef MVRE_GL_BUFFER_
#define MVRE_GL_BUFFER_

#include <MVRE/graphics/backend/template/buffer.hpp>
#include <glad/glad.h>

namespace mvre_graphics {

    class gl_buffer : public buffer {
    private:
        unsigned int m_buffer_id;
        int m_gl_type;
    public:
        using buffer::buffer;

        inline unsigned int id() const { return m_buffer_id; }

        void create(size_t _size, MVRE_MEMORY_TYPE _mem_type) override;

        void copy_data(void* data) override {
            switch (m_mem_type) {
                case MVRE_MEMORY_TYPE_UNIFORM:
                    bind();
                    glBufferSubData(m_gl_type, 0, static_cast<GLsizeiptr>(m_size), data);
                    unbind();
                    break;
                default:
                    glBufferData(m_gl_type, static_cast<GLsizeiptr>(m_size), data, GL_STATIC_DRAW);
                    break;
            }
        }

        void bind() override { glBindBuffer(m_gl_type, m_buffer_id); }
        void unbind() override { glBindBuffer(m_gl_type, 0); }
    };
}

#endif