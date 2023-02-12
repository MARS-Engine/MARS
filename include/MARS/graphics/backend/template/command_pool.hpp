#ifndef MARS_COMMAND_POOL_
#define MARS_COMMAND_POOL_

#include "graphics_component.hpp"

namespace mars_graphics {

    class command_pool : public graphics_component {
    private:

    public:
        using graphics_component::graphics_component;

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif