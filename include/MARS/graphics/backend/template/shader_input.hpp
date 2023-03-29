#ifndef MARS_SHADER_INPUT_
#define MARS_SHADER_INPUT_

#include "graphics_component.hpp"
#include "buffer.hpp"
#include <vector>

namespace mars_graphics {

    class shader_input : public graphics_component {
    protected:
        std::vector<buffer*> m_buffers;
    public:
        using graphics_component::graphics_component;

        virtual void create() { }

        virtual void bind() { }
        virtual void unbind() { }

        virtual void destroy() { }

        virtual buffer* add_buffer(size_t _input_size, MARS_MEMORY_TYPE _input_type) { return nullptr; }
        virtual void load_input(const std::shared_ptr<mars_shader_inputs>& _inputs) { }
    };
}

#endif