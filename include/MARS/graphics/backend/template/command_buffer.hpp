#ifndef MARS_COMMAND_BUFFER_
#define MARS_COMMAND_BUFFER_

#include "graphics_component.hpp"
#include "command_pool.hpp"
#include "builders/command_buffer_builder.hpp"

namespace mars_graphics {

    class command_buffer : public graphics_component {
    protected:
        size_t m_size;

        virtual void create() { }

        friend command_buffer_builder;
    public:
        using graphics_component::graphics_component;

        virtual void begin(size_t _i) { }
        virtual void end(size_t _i) { }
        virtual void reset(size_t _i) { }
        virtual void submit() { }

        virtual void draw(int first, size_t _count) { }
        virtual void draw_indexed(size_t _indices) { }
        virtual void draw_instanced(size_t _indices, size_t _instances) { }
    };
}

#endif