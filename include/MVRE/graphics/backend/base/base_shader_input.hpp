#ifndef __MVRE__BASE__SHADER__INPUT__
#define __MVRE__BASE__SHADER__INPUT__

#include <vector>
#include "base_buffer.hpp"

namespace mvre_graphics_base {

    class base_shader_input {
    protected:
        std::vector<base_buffer*> m_buffers;
    public:
        virtual void create() { }

        virtual void bind() { }
        virtual void unbind() { }

        virtual base_buffer* add_buffer(size_t _input_size, MVRE_MEMORY_TYPE _input_type) { return nullptr; }
        virtual void load_input(mvre_shader_inputs _inputs) { }
    };
}

#endif