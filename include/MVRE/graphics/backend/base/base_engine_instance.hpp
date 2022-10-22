#ifndef __MVRE__BASE__ENGINE__INSTANCE__
#define __MVRE__BASE__ENGINE__INSTANCE__

#include "base_context.hpp"

namespace mvre_graphics_base {

    class base_engine_instance {
    public:
        base_context* context;

        virtual void create_with_window(mvre_graphics::window& _window) {  }

        virtual void update() { }
        virtual void clear() { }
        virtual void clean() { }
    };
}

#endif