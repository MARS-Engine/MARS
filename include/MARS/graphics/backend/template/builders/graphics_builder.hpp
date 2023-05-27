#ifndef MARS_GRAPHICS_BUILDER_
#define MARS_GRAPHICS_BUILDER_

#include <memory>
#include "MARS/debug/debug.hpp"

namespace mars_graphics {
    class graphics_component;

    template<typename T> class graphics_builder {
    protected:
        std::shared_ptr<T> m_ref;
        bool m_built = false;
    protected:
        void base_build() {
            if (m_built)
                mars_debug::debug::error("MARS - Graphics Builder - Attempted to execute build twice in same builder");

            m_built = true;
        }
    public:
        graphics_builder() = delete;
        graphics_builder(const graphics_builder& _copy) = delete;
        graphics_builder(graphics_builder&& _move) noexcept = default;
        graphics_builder& operator=(const graphics_builder& _copy) = delete;
        graphics_builder& operator=(graphics_builder&& _move) noexcept = default;

        ~graphics_builder() {
            if (!m_built)
                mars_debug::debug::alert("MARS - Graphics Builder - Created Builder but didn't build");
        }

        explicit graphics_builder(const std::shared_ptr<T>& _ptr) {
            m_ref = _ptr;
        }
    };
}

#endif