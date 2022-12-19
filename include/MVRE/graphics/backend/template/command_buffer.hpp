#ifndef MVRE_COMMAND_BUFFER_
#define MVRE_COMMAND_BUFFER_

#include "graphics_component.hpp"

namespace mvre_graphics {

    class command_buffer : public graphics_component {
    public:
        using graphics_component::graphics_component;

        virtual void begin() { }
        virtual void end() { }
        virtual void reset() { }

        virtual void draw(int first, size_t _count) { }
        virtual void draw_indexed(size_t _indices) { }
    };
}

#endif