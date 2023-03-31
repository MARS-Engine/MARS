#ifndef MARS_ENGINE_OBJECT_
#define MARS_ENGINE_OBJECT_

#include <pl/safe_vector.hpp>
#include <pl/safe_deque.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include "MARS/memory/mars_ref.hpp"
#include <memory>

#include "transform_3d.hpp"
#include "component.hpp"
#include "bridge.hpp"

namespace mars_engine {
    class _object_engine;
    typedef std::shared_ptr<_object_engine> object_engine;

    class _mars_object : public std::enable_shared_from_this<_mars_object> {
    private:
        mars_object m_parent;

        mars_graphics::graphics_engine m_graphics;
        object_engine m_engine;

        pl::safe_deque<mars_object> m_children;
        pl::safe_vector<std::shared_ptr<component>> m_components;
        pl::safe_map<std::string, std::shared_ptr<bridge>> m_bridges;

        transform_3d m_transform;
    public:
        mars_object parent() const { return m_parent; }
        object_engine engine() const { return m_engine; }
        mars_graphics::graphics_engine graphics() const { return m_graphics; }
        transform_3d& transform() { return m_transform; }
        const pl::safe_vector<std::shared_ptr<component>>& components() const { return m_components; }

        explicit _mars_object(const object_engine& _engine) {
            m_engine = _engine;
            m_transform.set_parent(this);
        }

        void set_parent(const mars_object& _parent) {
            m_parent = _parent;
            m_parent->m_children.push_back(get_ptr());
        }

        void set_graphics(const mars_graphics::graphics_engine& _graphics) {
            m_graphics = _graphics;
        }

        [[nodiscard]] inline mars_object get_ptr() {
            return shared_from_this();
        }

        template<typename T> mars_mem::mars_ref<T> add_component(const std::shared_ptr<T>& _new_component) {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            m_components.lock();
            m_components.push_back(_new_component);
            m_components.unlock();
            _new_component->set_object(get_ptr());
            return mars_mem::mars_ref<T>(_new_component);
        }

        template<class T> inline mars_mem::mars_ref<T> add_component() { return add_component<T>(std::make_shared<T>()); }

        template<typename T> mars_mem::mars_ref<T> get_bridge(const std::string& _bridge) {
            static_assert(std::is_base_of_v<bridge, T>, "MARS - Engine Object - Invalid bridge, base must have type mars_engine::object_bridge");
            if (m_bridges.contains(_bridge))
                return mars_mem::mars_ref<bridge>(m_bridges[_bridge]).cast_static<T>();

            auto new_bridge = std::make_shared<T>(get_ptr());

            m_bridges.lock();

            if (m_bridges.contains(_bridge)) {
                m_bridges.unlock();
                return mars_mem::mars_ref<bridge>(m_bridges[_bridge]).cast_static<T>();
            }

            m_bridges.insert(std::pair(_bridge, new_bridge));
            m_bridges.unlock();

            return mars_mem::mars_ref<bridge>(new_bridge).cast_static<T>();
        }

        void destroy() {
            for (auto& component : m_components)
                component->destroy();

            for (auto& child : m_children)
                child->destroy();
        }
    };

    using mars_object = std::shared_ptr<_mars_object>;

    inline mars_object create_object(const object_engine& _engine) {
        return std::make_shared<_mars_object>(_engine);
    }
}

#endif