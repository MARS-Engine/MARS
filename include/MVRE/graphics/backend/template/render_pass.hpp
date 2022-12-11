#ifndef __MVRE__RENDER__PASS__
#define __MVRE__RENDER__PASS__

#include "graphics_component.hpp"

namespace mvre_graphics {

    class render_pass : public graphics_component {
    public:
        using graphics_component::graphics_component;

        virtual void begin() { }
        virtual void end() { }

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif