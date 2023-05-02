#ifndef MARS_GRAPHICS_COMPONENT_
#define MARS_GRAPHICS_COMPONENT_

#include "graphics_backend.hpp"
#include "graphics_types.hpp"

namespace mars_graphics {

    class graphics_component {
    private:
        //shared_ptr temporary workaround should be ref
        std::shared_ptr<graphics_backend> m_graphics;
    public:
        [[nodiscard]] inline std::shared_ptr<graphics_backend> graphics() const { return m_graphics; }
        template<typename T> inline T* cast_graphics() const { return static_cast<T*>(m_graphics.get()); }

        template<typename T> T* cast() { return static_cast<T*>(this); }
        template<typename T> const T* cast() const { return static_cast<T*>(this); }

        graphics_component() = delete;
        explicit graphics_component(const std::shared_ptr<graphics_backend>& _graphics) { m_graphics = _graphics; }
    };
}

#endif