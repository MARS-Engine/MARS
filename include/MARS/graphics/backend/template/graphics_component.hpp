#ifndef MARS_GRAPHICS_COMPONENT_
#define MARS_GRAPHICS_COMPONENT_

#include "backend_instance.hpp"
#include "graphics_types.hpp"

namespace mars_graphics {

    class graphics_component {
    private:
        backend_instance* m_instance;
    public:

        inline backend_instance* instance() const { return m_instance; }
        template<typename T> inline T* instance() const { return static_cast<T*>(m_instance); }

        explicit graphics_component(backend_instance* _instance) { m_instance = _instance; }
    };
}

#endif