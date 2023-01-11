#ifndef MARS_GL_BUFFER_
#define MARS_GL_BUFFER_

#include <MARS/graphics/backend/template/buffer.hpp>
#include <glad/glad.h>

namespace mars_graphics {

    class gl_buffer : public buffer {
    private:
        std::vector<unsigned int> m_ids;
        int m_gl_type;
    public:
        using buffer::buffer;

        unsigned int id();

        void create(size_t _size, MARS_MEMORY_TYPE _mem_type) override;

        void update(void* _data) override {
            m_current_data = _data;
        }

        void copy_data() override {
            switch (m_mem_type) {
                case MARS_MEMORY_TYPE_UNIFORM:
                    glBufferSubData(m_gl_type, 0, static_cast<GLsizeiptr>(m_size), m_current_data);
                    break;
                default:
                    bind();
                    glBufferData(m_gl_type, static_cast<GLsizeiptr>(m_size), m_current_data, GL_STATIC_DRAW);
                    break;
            }
        }

        void bind() override;
        void unbind() override { glBindBuffer(m_gl_type, 0); }
    };
}

#endif