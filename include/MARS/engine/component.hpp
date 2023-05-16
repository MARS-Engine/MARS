#ifndef MARS_COMPONENT_
#define MARS_COMPONENT_

#include "MARS/memory/mars_ref.hpp"
#include <MARS/graphics/graphics_engine.hpp>
#include "transform_3d.hpp"

namespace mars_engine {
    class mars_object;
    class object_engine;

    class component {
    private:
        mars_ref<mars_object> m_object;
    public:
        void set_object(const mars_ref<mars_object>& _parent);

        [[nodiscard]] mars_ref<object_engine> engine() const;
        [[nodiscard]] mars_ref<mars_graphics::graphics_engine> graphics() const;

        transform_3d& transform();

        [[nodiscard]] inline mars_ref<mars_object> object() const { return m_object; }

        virtual void on_set_object() { }

        virtual void destroy() { }
    };
}

#endif