#ifndef MARS_COMPONENT_
#define MARS_COMPONENT_

#include <MARS/graphics/graphics_engine.hpp>
#include "transform_3d.hpp"

namespace mars_engine {
    class mars_object;
    class object_engine;

    class component {
    private:
        std::shared_ptr<mars_object> m_object;
    public:
        void set_object(mars_object& _parent);

        [[nodiscard]] std::shared_ptr<object_engine> engine() const;
        [[nodiscard]] std::shared_ptr<mars_graphics::graphics_engine> graphics() const;

        transform_3d& transform();

        inline std::shared_ptr<mars_object> object() { return m_object; }
        [[nodiscard]] inline mars_input::input* get_input() const { return mars_input::input_manager::get_input(graphics()->backend()->get_window()); }

        virtual void on_set_object() { }

        virtual void destroy() { }
    };
}

#endif