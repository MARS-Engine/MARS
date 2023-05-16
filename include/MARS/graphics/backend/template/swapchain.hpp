#ifndef MARS_SWAPCHAIN_
#define MARS_SWAPCHAIN_

#include "graphics_component.hpp"

namespace mars_graphics {

    class swapchain : public graphics_component {
    protected:
        mars_math::vector2<uint32_t> m_size;
        size_t m_buffers;
    public:

        using graphics_component::graphics_component;

        [[nodiscard]] inline mars_math::vector2<uint32_t> size() const { return m_size; }
        [[nodiscard]] inline size_t buffers() const { return m_buffers; }

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif