#ifndef MARS_SYNC_
#define MARS_SYNC_

#include "graphics_component.hpp"

namespace mars_graphics {

    class sync : public graphics_component {
    public:
        using graphics_component::graphics_component;

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif