#ifndef __MVRE__BASE__CONTEXT__
#define __MVRE__BASE__CONTEXT__

#include "MVRE/graphics/window.hpp"

namespace mvre_graphics_base {

    class base_context {
    public:
        mvre_graphics::window& surf_window;

        explicit base_context(mvre_graphics::window& _window) : surf_window(_window) { }

        virtual void create() { }
        virtual void clean() { }
    };
}

#endif