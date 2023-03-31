#ifndef MARS_MARS_ENGINE_
#define MARS_MARS_ENGINE_

#include <deque>
#include <functional>
#include <typeindex>
#include <thread>
#include <pl/safe_map.hpp>
#include <pl/safe_vector.hpp>
#include <pl/safe_deque.hpp>
#include <MARS/resources/resource_manager.hpp>
#include "singleton.hpp"
#include "mars_object.hpp"

namespace mars_engine {
    class tick {
    private:
        std::chrono::_V2::system_clock::time_point m_last_tick = std::chrono::high_resolution_clock::now();
        float m_delta_time = 0.0001f;
        float m_delta_time_ms = 0.0001f;
    public:
        [[nodiscard]] float delta() const { return m_delta_time; }
        [[nodiscard]] float delta_ms() const { return m_delta_time_ms; }

        void exec_tick() {
            auto now = std::chrono::high_resolution_clock::now();
            m_delta_time_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - m_last_tick).count();
            m_delta_time = std::chrono::duration<float, std::chrono::seconds::period>(now - m_last_tick).count();
            m_last_tick = std::chrono::high_resolution_clock::now();
        }
    };

    class engine_layer_component;

    struct layer_component_param {
        engine_layer_component* component;
        tick* layer_tick;
        std::shared_ptr<std::vector<engine_layer_component>> layers;
        size_t index;
    };

    struct engine_layer_component {
        const mars_object* parent;
        void* target = nullptr;
        std::function<void(const mars_engine::layer_component_param&)> callback;
    };

    struct engine_layers {
        tick m_tick;
        std::function<std::vector<engine_layer_component>(const mars_object&)> m_validator;

        explicit engine_layers(const std::function<std::vector<engine_layer_component>(const mars_object&)>& _validator) {
            m_validator = _validator;
        }
    };

    class engine_worker;

    typedef std::shared_ptr<_object_engine> object_engine;

    class _object_engine : public std::enable_shared_from_this<_object_engine> {
    private:
        mars_resources::resource_manager m_resources;

        std::deque<std::shared_ptr<engine_worker>> m_workers;
        pl::safe_deque<mars_object> m_new_objects;
        pl::safe_deque<mars_object> m_objects;
        pl::safe_map<std::type_index, std::shared_ptr<std::vector<engine_layer_component>>>  m_layer_calls;
        pl::safe_map<std::type_index, engine_layers> m_layer_data;
        pl::safe_map<std::type_index, std::shared_ptr<singleton>> m_singletons;
    public:
        object_engine get_ptr() { return shared_from_this(); }

        inline void set_resources(const mars_resources::resource_manager& _resource_manager) { m_resources = _resource_manager; }

        mars_resources::resource_manager resources() { return m_resources; }

        std::shared_ptr<std::vector<engine_layer_component>> get_components(std::type_index _layer) {
            if (!m_layer_calls.contains(_layer))
                return nullptr;
            return m_layer_calls.at(_layer);
        }

        engine_layers* get_layer(std::type_index _layer) {
            if (!m_layer_data.contains(_layer))
                return nullptr;

            return &m_layer_data.at(_layer);
        }

        std::shared_ptr<engine_worker> create_worker(size_t _cores);

        template<typename T> std::shared_ptr<T> get_or_create_singleton() {
            static_assert(std::is_base_of_v<singleton, T>, "MARS - Engine Object - Invalid singleton, base must have type mars_engine::singleton");

            auto type_index = std::type_index(typeid(T));
            if (m_singletons.contains(type_index))
                return std::static_pointer_cast<T>(m_singletons.at(type_index));

            m_singletons.lock();

            if (m_singletons.contains(type_index)) {
                m_singletons.unlock();
                return std::static_pointer_cast<T>(m_singletons.at(type_index));
            }

            std::shared_ptr<T> new_ptr = std::make_shared<T>(get_ptr());
            m_singletons.insert(std::pair(type_index, new_ptr));

            m_singletons.unlock();
            return new_ptr;
        }

        template<typename T> inline void add_layer(const std::function<std::vector<engine_layer_component>(const mars_object&)>& _validator) {
            auto type_index = std::type_index(typeid(T));
            m_layer_data.insert(std::make_pair(type_index, engine_layers(_validator)));
            m_layer_calls.insert(std::pair(type_index, std::make_shared<std::vector<engine_layer_component>>()));
        }

        void spawn_wait_list();
        void process_layers(const mars_object& _obj);

        mars_object spawn(const mars_object& _obj, const mars_graphics::graphics_engine& _graphics, const mars_object& _parent);

        mars_object spawn(const mars_object& _obj, const mars_graphics::graphics_engine& _graphics) {
            return spawn(_obj, _graphics, nullptr);
        }

        void clean() {
            for (auto& object : m_objects)
                object->destroy();
        }
    };

    inline object_engine create_engine() {
        return std::make_shared<_object_engine>();
    }
}

#endif