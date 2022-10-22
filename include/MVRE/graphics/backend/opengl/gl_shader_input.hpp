#ifndef __MVRE__GL__SHADER__INPUT__
#define __MVRE__GL__SHADER__INPUT__

#include "MVRE/graphics/backend/base/base_shader_input.hpp"

#include <glad/glad.h>

namespace mvre_graphics_opengl {

    class gl_shader_input : public mvre_graphics_base::base_shader_input {
    private:
        unsigned int m_id;
    public:
        void create() override;

        mvre_graphics_base::base_buffer* add_buffer(size_t _input_size, mvre_graphics_base::MVRE_MEMORY_TYPE _input_type) override;

        void load_input(mvre_graphics_base::mvre_shader_inputs _inputs) override;

        inline void bind() override { glBindVertexArray(m_id); }
        inline void unbind() override { glBindVertexArray(0); }
    };
    
}

#endif