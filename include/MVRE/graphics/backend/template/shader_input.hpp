#ifndef __MVRE__SHADER__INPUT__
#define __MVRE__SHADER__INPUT__

#include "graphics_component.hpp"
#include "buffer.hpp"

namespace mvre_graphics {

    class shader_input : public graphics_component {
    protected:
        std::vector<buffer*> m_buffers;
    public:
        using graphics_component::graphics_component;

        virtual void create() { }

        virtual void bind() { }
        virtual void unbind() { }

        virtual buffer* add_buffer(size_t _input_size, MVRE_MEMORY_TYPE _input_type) { return nullptr; }
        virtual void load_input(mvre_shader_inputs _inputs) { }
    };
}

#endif