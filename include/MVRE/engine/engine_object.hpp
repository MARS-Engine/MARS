#ifndef __MVRE__ENGINE__OBJECT__
#define __MVRE__ENGINE__OBJECT__

#include <pl/safe_vector.hpp>

#include <MVRE/graphics/graphics_instance.hpp>

namespace mvre_engine {

    class transform_3d;
    template<class T> class component;
    class component_interface;

    class engine_object {
    private:
        engine_object* m_parent = nullptr;
        mvre_graphics::graphics_instance* m_instance = nullptr;
        transform_3d* m_transform;
        pl::safe_vector<engine_object*> m_children;
        pl::safe_vector<component_interface*> m_components;
    public:
        inline engine_object* parent() { return m_parent; }
        inline transform_3d* transform() { return m_transform; }
        inline pl::safe_vector<engine_object*>& children() { return m_children; }
        inline pl::safe_vector<component_interface*>& components() { return m_components; }


        inline void set_instance(mvre_graphics::graphics_instance* _new_instance) { m_instance = _new_instance; }
        inline mvre_graphics::graphics_instance* instance() { return m_instance; }

        inline void set_parent(engine_object* _parent) { m_parent = _parent; m_parent->m_children.push_back(this); }

        engine_object();
        ~engine_object();

        template<class T> T* get_component() {
            static_assert(std::is_base_of<component<T>, T>::value, "invalid component - component must have type mvre_engine::component has a base");
            for (auto c : m_components) {
                auto cast = dynamic_cast<T*>(c);
                if (cast != nullptr)
                    return cast;
            }
        }

        template<class T> T* add_component(T* _new_component) {
            static_assert(std::is_base_of<component<T>, T>::value, "invalid component - component must have type mvre_engine::component has a base");
            m_components.push_back(_new_component);
            _new_component->set_object(this);
            return _new_component;
        }

        template<class T> inline T* add_component() { return add_component<T>(new T()); }
    };
};

#endif