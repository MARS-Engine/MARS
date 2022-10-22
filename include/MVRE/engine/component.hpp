#ifndef __MVRE__COMPONENT__
#define __MVRE__COMPONENT__

#include <type_traits>
#include <cstddef>

#include "engine_object.hpp"
#include "MVRE/executioner/executioner.hpp"

namespace mvre_engine {

    class component_interface {
    protected:
        bool m_is_renderer = false;
        engine_object* m_object;
    public:
        mvre_executioner::executioner_job* render_job = nullptr;

        void set_object(engine_object* _new_object) { m_object = _new_object; }

        ~component_interface() {
            if (render_job != nullptr)
                delete render_job;
        }

        virtual std::size_t size() const { return 0; }
        virtual void pre_load() { }
        virtual void load() { }
        virtual void pre_render() { }
        virtual void update() { }
        virtual void post_render() { }
        virtual void clean() { }
    };

    template<class derived> class component : public component_interface{
    public:
        component() {
            static_assert(std::is_base_of<component, derived>::value, "derived must be derived of component");
        }

        std::size_t size() const override { return sizeof(derived); }

        template<class T> inline T* get_component() { return m_object->get_component<T>(); }
        template<class T> inline T* add_component(T* comp) { return m_object->add_component(comp); }
        template<class T> inline T* add_component() { return m_object->add_component<T>(); }
    };
}

#endif