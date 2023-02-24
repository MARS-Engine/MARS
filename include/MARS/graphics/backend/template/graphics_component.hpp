#ifndef MARS_GRAPHICS_COMPONENT_
#define MARS_GRAPHICS_COMPONENT_

#include "graphics_backend.hpp"
#include "graphics_types.hpp"

namespace mars_graphics {

    class graphics_component {
    private:
        graphics_backend* m_graphics;
    public:

        [[nodiscard]] inline graphics_backend* graphics() const { return m_graphics; }
        template<typename T> inline T* cast_graphics() const { return static_cast<T*>(m_graphics); }

        explicit graphics_component(graphics_backend* _graphics) { m_graphics = _graphics; }
    };
}

#endif