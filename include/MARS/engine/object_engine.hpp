#ifndef MARS_MARS_ENGINE_
#define MARS_MARS_ENGINE_

#include <deque>
#include <functional>
#include <typeindex>
#include <thread>
#include "component.hpp"
#include <pl/safe_map.hpp>
#include <pl/safe_vector.hpp>
#include <pl/safe_deque.hpp>
#include <MARS/resources/resource_manager.hpp>
#include "singleton.hpp"

namespace mars_graphics {
    class graphics_engine;
}

namespace mars_engine {
    class mars_object;
    class component;

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
    class engine_worker;

    struct layer_component_param {
        engine_worker* _worker;
        tick* layer_tick;
        std::shared_ptr<std::vector<engine_layer_component>> layers;
        long being;
        long length;
    };

    struct engine_layer_component {
        void* target = nullptr;
        mars_object* parent = nullptr;

        engine_layer_component() = default;

        engine_layer_component(const engine_layer_component& _copy) noexcept {
            target = _copy.target;
            parent = _copy.parent;
        }

        engine_layer_component(engine_layer_component&& _move) noexcept {
            target = _move.target;
            parent = _move.parent;
        }

        engine_layer_component& operator=(const engine_layer_component& _copy) noexcept {
            if (this == &_copy)
                return *this;

            target = _copy.target;
            parent = _copy.parent;
            return *this;
        }

        engine_layer_component& operator=(engine_layer_component&& _move) noexcept {
            if (this == &_move)
                return *this;

            target = _move.target;
            parent = _move.parent;
            return *this;
        }
    };

    struct engine_layers {
        tick m_tick;
        void (*m_callback)(layer_component_param&&);
        std::function<bool(const mars_ref<component>&, engine_layer_component&)> m_validator;
        bool m_single_time;

        explicit engine_layers(const std::function<bool(const mars_ref<component>&, engine_layer_component&)>& _validor, void (*_callback)(layer_component_param&&), bool _single_time = false) {
            m_validator = _validor;
            m_callback = _callback;
            m_single_time = _single_time;
        }
    };

    class object_engine : public std::enable_shared_from_this<object_engine> {
    private:
        mars_ref<mars_resources::resource_manager> m_resources;
        mars_ref<mars_graphics::graphics_engine> m_graphics;

        std::deque<std::shared_ptr<engine_worker>> m_workers;

        pl::safe_deque<std::shared_ptr<mars_object>> m_destroy_list;
        pl::safe_deque<std::shared_ptr<mars_object>> m_objects;
        pl::safe_map<std::type_index, engine_layers> m_layer_data;

        std::map<std::type_index, std::shared_ptr<std::vector<engine_layer_component>>> m_wait_list;
        std::map<std::type_index, std::shared_ptr<std::vector<engine_layer_component>>> m_layer_calls;

        pl::safe_map<std::type_index, std::shared_ptr<singleton>> m_singletons;
        std::barrier<std::function<void()>> m_spawn_wait;
        std::atomic<bool> layers_waiting = false;
    public:
        explicit object_engine(size_t _active_workers) : m_spawn_wait(_active_workers, [&]() { spawn_wait_list(); layers_waiting.exchange(false); }) { }

        std::shared_ptr<object_engine> get_ptr() { return shared_from_this(); }

        inline void set_graphics(const mars_ref<mars_graphics::graphics_engine>& _graphics) { m_graphics = _graphics; }

        [[nodiscard]] mars_ref<mars_graphics::graphics_engine> graphics() const { return m_graphics; }

        inline void set_resources(const mars_ref<mars_resources::resource_manager>& _resource_manager) { m_resources = _resource_manager; }

        [[nodiscard]]  mars_ref<mars_resources::resource_manager> resources() { return m_resources; }

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

        void clear_layer(std::type_index _layer) {
            if (m_layer_calls.contains(_layer))
                m_layer_calls.at(_layer)->clear();
        }

        std::shared_ptr<engine_worker> create_worker(size_t _cores);

        mars_ref<mars_object> create_obj();

        template<typename T> std::shared_ptr<T> get_singleton() {
            static_assert(std::is_base_of_v<singleton, T>, "MARS - Engine Object - Invalid singleton, base must have type mars_engine::singleton");

            auto type_index = std::type_index(typeid(T));
            if (m_singletons.contains(type_index))
                return std::static_pointer_cast<T>(m_singletons.at(type_index));

            m_singletons.lock();

            if (m_singletons.contains(type_index)) {
                m_singletons.unlock();
                return std::static_pointer_cast<T>(m_singletons.at(type_index));
            }

            std::shared_ptr<T> new_ptr = std::make_shared<T>();
            new_ptr->set_engine(mars_ref<object_engine>(get_ptr()));
            m_singletons.insert(std::pair(type_index, new_ptr));

            m_singletons.unlock();
            return new_ptr;
        }

        template<typename T> inline void add_layer(void (*_callback)(layer_component_param&&), bool _single_time = false) {
            auto _validator = [](const mars_ref<mars_engine::component>& _target, mars_engine::engine_layer_component& _val) {
                auto target = dynamic_cast<T*>(_target.ptr());

                if (target == nullptr)
                    return false;

                _val.target = target;
                _val.parent = _target->object().ptr();

                return true;
            };


            auto type_index = std::type_index(typeid(T));
            m_layer_data.insert(std::make_pair(type_index, engine_layers(_validator, _callback, _single_time)));
            m_layer_calls.insert(std::pair(type_index, std::make_shared<std::vector<engine_layer_component>>()));
            m_wait_list.insert(std::make_pair(type_index, std::make_shared<std::vector<engine_layer_component>>()));
        }

        void spawn_wait_list();

        void spawn_wait_room() {
            if (layers_waiting)
                m_spawn_wait.arrive_and_wait();
        }

        void process_component(const mars_ref<component>& _component);

        mars_ref<mars_object> spawn(const mars_ref<mars_object>& _obj, const mars_ref<mars_graphics::graphics_engine>& _graphics, const mars_ref<mars_object>& _parent);

        mars_ref<mars_object> spawn(const mars_ref<mars_object>& _obj, const mars_ref<mars_graphics::graphics_engine>& _graphics) {
            return spawn(_obj, _graphics, mars_ref<mars_object>());
        }

        void clean();
    };
}

#endif