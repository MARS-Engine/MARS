#ifndef MARS_ENGINE_OBJECT_
#define MARS_ENGINE_OBJECT_

#include <pl/safe_vector.hpp>

#include <MARS/graphics/graphics_instance.hpp>

namespace mars_engine {

    class transform_3d;
    class component;

    class engine_object {
    private:
        engine_object* m_parent = nullptr;
        mars_graphics::graphics_instance* m_instance = nullptr;
        transform_3d* m_transform;
        pl::safe_vector<engine_object*> m_children;
        pl::safe_vector<component*> m_components;
    public:
        inline engine_object* parent() const { return m_parent; }
        inline transform_3d* transform() const { return m_transform; }
        inline pl::safe_vector<engine_object*>& children() { return m_children; }
        inline pl::safe_vector<component*>& components() { return m_components; }


        void set_instance(mars_graphics::graphics_instance* _new_instance) {
            m_instance = _new_instance;

            //TODO: Make non-recursive
            for (auto i : m_children)
                i->set_instance(m_instance);
        }


        inline mars_graphics::graphics_instance* instance() const { return m_instance; }

        inline void set_parent(engine_object* _parent) { m_parent = _parent; m_parent->m_children.push_back(this); }

        engine_object();
        ~engine_object();

        template<class T> T* get_component() const {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            for (auto c : m_components) {
                auto cast = dynamic_cast<T*>(c);
                if (cast != nullptr)
                    return cast;
            }
        }

        template<class T> T* add_component(T* _new_component) {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            m_components.push_back(_new_component);
            _new_component->set_object(this);
            return _new_component;
        }

        template<class T> inline T* add_component() { return add_component<T>(new T()); }
    };
};

#endif