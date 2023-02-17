#ifndef MARS_COMPONENT_
#define MARS_COMPONENT_

#include <type_traits>
#include <cstddef>

#include <MARS/executioner/executioner.hpp>
#include "engine_object.hpp"
#include "transform_3d.hpp"

namespace mars_engine {

    class component {
    protected:
        engine_object* m_object = nullptr;
    public:

        inline mars_input::input* get_input() { return mars_input::input_manager::get_input(g_instance()->backend()->get_window()); }
        inline transform_3d& transform() { return m_object->transform(); }
        inline engine_object* object() { return m_object; }
        inline mars_graphics::graphics_instance* g_instance() { return m_object->instance(); }

        float get_delta_time();
        float get_delta_time_ms();

        void set_object(engine_object* _new_object) { m_object = _new_object; }

        virtual void destroy() { }

        template<class T> inline T* get_component() { return m_object->get_component<T>(); }
        template<class T> inline T* add_component(T* comp) { return m_object->add_component(comp); }
        template<class T> inline T* add_component() { return m_object->add_component<T>(); }
    };
}

#endif