#ifndef MARS_ENGINE_HANDLER_
#define MARS_ENGINE_HANDLER_

#include <pl/parallel.hpp>
#include <pl/safe.hpp>
#include <MARS/graphics/graphics_engine.hpp>
#include <MARS/resources/resource_manager.hpp>
#include <vector>
#include <typeindex>
#include <optional>
#include "engine_object.hpp"
#include "component.hpp"
#include "singleton.hpp"
#include "engine_worker.hpp"

namespace mars_engine {

    struct engine_layer_component {
    private:
        std::atomic<bool> m_completed = false;
    public:
        engine_layer_component* next = nullptr;
        engine_object* parent = nullptr;
        void* target = nullptr;
        std::function<void(engine_layer_component*)> callback;

        inline bool completed(bool _old) { return m_completed.exchange(_old); }

        inline void clear() { m_completed = false; }
    };

    struct engine_layers {
        std::function<std::vector<engine_layer_component*>(engine_object*)> validator;
        std::chrono::_V2::system_clock::time_point _last_time;
        float delta_time  = 0.0001f;
        float delta_time_ms  = 0.0001f;
    };

    class engine_handler {
    private:
        pl::safe_vector<engine_worker*> m_workers;
        pl::safe<engine_object*> m_objects = nullptr;
        pl::safe<engine_object*> m_new_objects = nullptr;
        pl::safe_map<std::type_index, engine_layers*> layer_data;
        std::type_index m_layer_index = std::type_index(typeid(engine_handler));
        pl::safe_map<std::type_index, std::vector<engine_layer_component*>> m_layer_components;
        pl::safe_map<std::type_index, singleton*> m_singletons;
        mars_resources::resource_manager* m_resources;

        size_t m_active_cores;
        size_t m_next_core;
        std::mutex m_layer_mtx;

        static void callback(const pl::safe_vector<engine_layer_component*>& _components);
        void process_layers(engine_object* _obj);
    public:
        [[nodiscard]] inline std::vector<engine_layer_component*>& get_current_components() { return m_layer_components[m_layer_index]; }
        [[nodiscard]] inline mars_resources::resource_manager* resources() const { return m_resources; }
        inline void set_resources(mars_resources::resource_manager* _resource_manager) { m_resources = _resource_manager; }

        [[nodiscard]] inline float get_delta_time() { return layer_data[m_layer_index]->delta_time; }
        [[nodiscard]] inline float get_delta_time_ms() { return layer_data[m_layer_index]->delta_time_ms; }

        template<typename T> inline engine_layers* get_layer() {
            return layer_data[typeid(T)];
        }

        template<typename T> inline void add_layer(const std::function<std::vector<engine_layer_component*>(engine_object*)>& _validator) {
            auto type_index = std::type_index(typeid(T));
            layer_data.insert(std::make_pair(type_index, new engine_layers{ .validator = _validator, ._last_time = std::chrono::high_resolution_clock::now() }));
            m_layer_components.insert(std::pair(type_index, std::vector<engine_layer_component*>()));
        }

        template<typename T> inline T* get_or_create_singleton() {
            static_assert(std::is_base_of_v<singleton, T>, "MARS - Engine - Failed to create singleton, singleton must have mars_engine::singleton as base");
            auto type_index = std::type_index(typeid(T));

            if (m_singletons.contains(type_index))
                return (T*)m_singletons[type_index];

            auto new_singleton = new T(this);
            m_singletons.insert(std::pair(type_index, new_singleton));
            return new_singleton;
        }

        void init();

        inline void lock() {
            m_layer_mtx.lock();
        }

        inline void unlock() {
            m_layer_mtx.unlock();
        }

        template<typename T> inline void process_layer() {
            m_layer_index = std::type_index(typeid(T));

            //Updating
            for (auto& worker : m_workers)
                worker->execute();
            for (auto& worker : m_workers)
                worker->wait();
            if (!m_layer_components[m_layer_index].empty()) {
                for (auto& component : m_layer_components[m_layer_index])
                    component->clear();
            }

            //Timing
            auto now = std::chrono::high_resolution_clock::now();
            layer_data[m_layer_index]->delta_time = std::chrono::duration<float, std::chrono::seconds::period>(now - layer_data[m_layer_index]->_last_time).count();
            layer_data[m_layer_index]->delta_time_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - layer_data[m_layer_index]->_last_time).count();
            layer_data[m_layer_index]->_last_time = now;
        }

        void clean();

        void spawn_wait_list();

        engine_object* spawn(engine_object* _obj, mars_graphics::graphics_engine* _graphics, engine_object* _parent);
        engine_object* spawn(engine_object* _obj, engine_object* _parent);
    };
}

#endif