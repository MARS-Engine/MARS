#ifndef __MVRE__BACKEND__BASE__
#define __MVRE__BACKEND__BASE__

#include <MVRE/graphics/engine_instance.hpp>

namespace mvre_graphics_base {

    class backend_base {
    protected:
        mvre_graphics::engine_instance* instance = nullptr;
    public:
        inline void set_instance(mvre_graphics::engine_instance* _instance) { instance = _instance; }

        backend_base(mvre_graphics::engine_instance* _instance) { instance = _instance; }

        inline virtual void load() { }
        inline virtual void clean() { }
    };
}

#endif