#ifndef MARS_COMPONENT_
#define MARS_COMPONENT_

#include "MARS/memory/mars_ref.hpp"
#include <MARS/graphics/graphics_engine.hpp>
#include "transform_3d.hpp"

namespace mars_engine {
    class _mars_object;
    typedef std::shared_ptr<_mars_object> mars_object;

    class _object_engine;
    typedef std::shared_ptr<_object_engine> object_engine;

    class component {
    private:
        mars_object m_object;
    public:
        void set_object(const mars_object& _parent);

        [[nodiscard]] object_engine engine() const;
        [[nodiscard]] mars_graphics::graphics_engine graphics() const;

        transform_3d& transform();

        inline mars_object object() { return m_object; }
        [[nodiscard]] inline mars_input::input* get_input() const { return mars_input::input_manager::get_input(graphics()->backend()->get_window()); }

        virtual void on_set_object() { }

        virtual void destroy() { }
    };
}

#endif