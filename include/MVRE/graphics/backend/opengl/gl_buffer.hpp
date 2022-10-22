#ifndef __MVRE__GL__BUFFER__
#define __MVRE__GL__BUFFER__

#include "MVRE/graphics/backend/base/base_buffer.hpp"

#include <glad/glad.h>

namespace mvre_graphics_opengl {

    class gl_buffer : public mvre_graphics_base::base_buffer {
    private:
        unsigned int m_buffer_id;
        int m_gl_type;
    public:
        inline unsigned int get_id() { return m_buffer_id; }

        void create(size_t _size, mvre_graphics_base::MVRE_MEMORY_TYPE _mem_type) override;

        void copy_data(void* data) override { glBufferData(m_gl_type, m_size, data, GL_STATIC_DRAW); }

        void bind() override { glBindBuffer(m_gl_type, m_buffer_id); }
        void unbind() override { glBindBuffer(m_gl_type, 0); }
    };
}

#endif