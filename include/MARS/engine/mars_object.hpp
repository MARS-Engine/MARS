#ifndef MARS_ENGINE_OBJECT_
#define MARS_ENGINE_OBJECT_

#include <pl/safe_vector.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include <memory>

#include "transform_3d.hpp"
#include "component.hpp"
#include "bridge.hpp"

namespace mars_engine {
    class object_engine;

    class mars_object : public std::enable_shared_from_this<mars_object> {
    private:
        std::shared_ptr<mars_object> m_parent;

        std::shared_ptr<mars_graphics::graphics_engine> m_graphics;
        std::shared_ptr<object_engine> m_engine;

        pl::safe_vector<std::shared_ptr<component>> m_components;
        pl::safe_map<std::string, std::shared_ptr<bridge>> m_bridges;

        transform_3d m_transform;
    public:
        std::shared_ptr<mars_object> parent() const { return m_parent; }
        std::shared_ptr<object_engine> engine() const { return m_engine; }
        std::shared_ptr<mars_graphics::graphics_engine> graphics() const { return m_graphics; }
        transform_3d& transform() { return m_transform; }
        const pl::safe_vector<std::shared_ptr<component>>& components() const { return m_components; }

        explicit mars_object(const std::shared_ptr<mars_engine::object_engine>& _engine) {
            m_engine = _engine;
            m_transform.set_parent(this);
        }

        void set_graphics(const std::shared_ptr<mars_graphics::graphics_engine>& _graphics) {
            m_graphics = _graphics;
        }

        [[nodiscard]] inline std::shared_ptr<mars_object> get_ptr() {
            return shared_from_this();
        }

        template<typename T> std::shared_ptr<T> add_component(std::shared_ptr<T> _new_component) {
            static_assert(std::is_base_of<component, T>::value, "invalid component - component must have type mars_engine::component has a base");
            m_components.lock();
            m_components.push_back(_new_component);
            m_components.unlock();
            _new_component->set_object(*this);
            return _new_component;
        }

        template<class T> inline std::shared_ptr<T> add_component() { return add_component<T>(std::make_shared<T>()); }

        template<typename T> std::shared_ptr<T> get_bridge(const std::string& _bridge) {
            static_assert(std::is_base_of_v<bridge, T>, "MARS - Engine Object - Invalid bridge, base must have type mars_engine::object_bridge");
            if (m_bridges.contains(_bridge))
                return std::static_pointer_cast<T>(m_bridges[_bridge]);

            auto new_bridge = std::make_shared<T>(*this);

            m_bridges.lock();

            if (m_bridges.contains(_bridge)) {
                m_bridges.unlock();
                return std::static_pointer_cast<T>(m_bridges[_bridge]);
            }

            m_bridges.insert(std::pair(_bridge, new_bridge));
            m_bridges.unlock();

            return new_bridge;
        }
    };
}

#endif