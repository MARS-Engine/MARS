#ifndef MARS_ENGINE_OBJECT_
#define MARS_ENGINE_OBJECT_

#include <pl/safe_vector.hpp>
#include <pl/safe_deque.hpp>
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

        pl::safe_deque<mars_ref<mars_object>> m_children;
        pl::safe_vector<std::shared_ptr<component>> m_components;
        pl::safe_map<std::string, std::shared_ptr<bridge>> m_bridges;

        transform_3d m_transform;
    public:
        mars_ref<mars_object> parent() const { return m_parent; }
        mars_ref<object_engine> engine() const { return m_engine; }
        transform_3d& transform() { return m_transform; }
        const pl::safe_vector<std::shared_ptr<component>>& components() const { return m_components; }

        inline void set_engine(const mars_ref<object_engine>& _engine) { m_engine = _engine; }

        explicit mars_object(const mars_ref<object_engine>& _engine) {
            m_engine = _engine;
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

        template<typename T> mars_ref<T> get_bridge(const std::string& _bridge) {
            static_assert(std::is_base_of_v<bridge, T>, "MARS - Engine Object - Invalid bridge, base must have type mars_engine::object_bridge");
            if (m_bridges.contains(_bridge))
                return mars_ref<bridge>(m_bridges[_bridge]).cast_static<T>();

            auto new_bridge = std::make_shared<T>(mars_ref<mars_object>(get_ptr()));

            m_bridges.lock();

            if (m_bridges.contains(_bridge)) {
                m_bridges.unlock();
                return mars_ref<bridge>(m_bridges[_bridge]).cast_static<T>();
            }

            m_bridges.insert(std::pair(_bridge, new_bridge));
            m_bridges.unlock();

            return mars_ref<bridge>(new_bridge).cast_static<T>();
        }

        void destroy() {
            for (auto& component : *m_components.lock().get())
                component->destroy();

            for (auto& child : *m_children.lock().get())
                child->destroy();
        }
    };
}

#endif