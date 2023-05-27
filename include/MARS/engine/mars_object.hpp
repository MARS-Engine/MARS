#ifndef MARS_ENGINE_OBJECT_
#define MARS_ENGINE_OBJECT_

#include <pl/safe.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include "MARS/memory/mars_ref.hpp"
#include <memory>

#include "object_engine.hpp"
#include "transform_3d.hpp"
#include "component.hpp"
#include "bridge.hpp"

namespace mars_engine {
    class object_engine;

    class mars_object : public std::enable_shared_from_this<mars_object> {
    private:
        mars_ref<mars_object> m_parent;

        mars_ref<object_engine> m_engine;

        pl::safe<std::deque<mars_ref<mars_object>>> m_children;
        pl::safe<std::vector<std::shared_ptr<component>>> m_components;
        pl::safe<std::map<std::string, std::shared_ptr<bridge>>> m_bridges;

        transform_3d m_transform;

        inline void set_engine(const mars_ref<object_engine>& _engine) { m_engine = _engine; }

        friend object_engine;
    public:
        mars_ref<mars_object> parent() const { return m_parent; }
        mars_ref<object_engine> engine() const { return m_engine; }
        transform_3d& transform() { return m_transform; }
        const pl::safe<std::vector<std::shared_ptr<component>>>& components() const { return m_components; }

        explicit mars_object() {
            m_transform.set_parent(this);
        }

        void set_parent(const mars_ref<mars_object>& _parent) {
            m_parent = _parent;
            m_parent->m_children.lock()->push_back(mars_ref<mars_object>(get_ptr()));
        }

        [[nodiscard]] inline std::shared_ptr<mars_object> get_ptr() {
            return shared_from_this();
        }

        template<typename T> mars_ref<T> add_component(const std::shared_ptr<T>& _new_component) {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            m_components.lock()->push_back(_new_component);
            _new_component->set_object(mars_ref<mars_object>(get_ptr()));
            m_engine->process_component(mars_ref<component>(_new_component));
            return mars_ref<T>(_new_component);
        }

        template<class T> inline mars_ref<T> add_component() { return add_component<T>(std::make_shared<T>()); }

        template<typename T> requires std::is_base_of_v<bridge, T> mars_ref<T> get(const std::string& _bridge) {
            auto locked_bridge = m_bridges.lock();

            if (locked_bridge->contains(_bridge))
                return mars_ref<bridge>(locked_bridge->at(_bridge)).cast_static<T>();

            auto new_bridge = std::make_shared<T>();
            new_bridge->set_object(mars_ref<mars_object>(get_ptr()));

            if (locked_bridge->contains(_bridge))
                return mars_ref<bridge>(locked_bridge->at(_bridge)).cast_static<T>();

            locked_bridge->insert(std::pair(_bridge, new_bridge));

            return mars_ref<bridge>(new_bridge).cast_static<T>();
        }

        ~mars_object() {
            for (auto& component : *m_components.lock().get())
                component->destroy();
            m_components.lock()->clear();
            m_children.lock()->clear();
            m_bridges.lock()->clear();
        }
    };
}

#endif