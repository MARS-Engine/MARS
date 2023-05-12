#ifndef MARS_GRAPHICS_BUILDER_
#define MARS_GRAPHICS_BUILDER_

#include <memory>
#include "MARS/debug/debug.hpp"

namespace mars_graphics {
    class graphics_component;

    template<typename T> class graphics_builder {
    protected:
        std::shared_ptr<T> m_ref;
        bool built = false;
    public:
        graphics_builder() = delete;

        explicit graphics_builder(const std::shared_ptr<T>& _ptr) {
            m_ref = _ptr;
        }

        void base_build() {
            if (!built) {
                built = true;
                return;
            }

            mars_debug::debug::error("MARS - Graphics Builder - Attempted to execute build twice in same builder");
        }
    };
}

#endif