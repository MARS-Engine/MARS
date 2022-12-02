#ifndef __MVRE__COMPONENT__
#define __MVRE__COMPONENT__

#include <type_traits>
#include <cstddef>

#include "engine_object.hpp"
#include "MVRE/executioner/executioner.hpp"
#include "transform_3d.hpp"

namespace mvre_engine {

    class component_interface {
    protected:
        engine_object* m_object;
    public:
        mvre_executioner::executioner_job* update_job = nullptr;

        void set_object(engine_object* _new_object) { m_object = _new_object; }

        virtual std::size_t size() const { return 0; }
    };

    template<class derived> class component : public component_interface{
    public:
        component() {
            static_assert(std::is_base_of<component, derived>::value, "derived must be derived of component");
        }

        inline transform_3d* transform() { return m_object->transform(); }
        inline engine_object* object() { return m_object; }
        inline mvre_graphics::graphics_instance* g_instance() { return m_object->instance(); }

        std::size_t size() const override { return sizeof(derived); }

        template<class T> inline T* get_component() { return m_object->get_component<T>(); }
        template<class T> inline T* add_component(T* comp) { return m_object->add_component(comp); }
        template<class T> inline T* add_component() { return m_object->add_component<T>(); }
    };
}

#endif