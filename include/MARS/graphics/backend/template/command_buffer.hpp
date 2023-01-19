#ifndef MARS_COMMAND_BUFFER_
#define MARS_COMMAND_BUFFER_

#include "graphics_component.hpp"

namespace mars_graphics {

    class command_buffer : public graphics_component {
    public:
        using graphics_component::graphics_component;

        virtual void begin() { }
        virtual void end() { }
        virtual void reset() { }

        virtual void draw(int first, size_t _count) { }
        virtual void draw_indexed(size_t _indices) { }
        virtual void draw_instanced(size_t _indices, size_t _instances) { }
    };
}

#endif