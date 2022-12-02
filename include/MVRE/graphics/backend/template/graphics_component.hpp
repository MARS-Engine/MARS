#ifndef __MVRE__GRAPHICS__COMPONENT__
#define __MVRE__GRAPHICS__COMPONENT__

#include "backend_instance.hpp"

namespace mvre_graphics {

    class graphics_component {
    private:
        backend_instance* m_instance;
    public:

        inline backend_instance* instance() { return m_instance; }

        graphics_component(backend_instance* _instance) { m_instance = _instance; }
    };
}

#endif