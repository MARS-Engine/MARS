#ifndef MARS_ENGINE_HANDLER_
#define MARS_ENGINE_HANDLER_

#include <pl/parallel.hpp>
#include <pl/safe.hpp>
#include <MARS/graphics/graphics_instance.hpp>
#include <vector>
#include <typeindex>
#include "engine_object.hpp"
#include "component.hpp"
#include "singleton.hpp"

namespace mars_engine {

    struct engine_layer_component {
    private:
        std::atomic<bool> m_completed = false;
    public:
        inline bool completed(bool _old) { return m_completed.exchange(_old); }

        engine_layer_component* next;
        engine_layer_component* previous;
        engine_object* parent;

        void* target;

        std::function<void(engine_layer_component*)> callback;

        void clear() {
            m_completed = false;
            auto next_clear = next;

            while (next_clear != nullptr) {
                next_clear->m_completed = false;
                next_clear = next_clear->next;
            }
        }
    };

    struct engine_layers {
        std::function<std::pair<engine_layer_component*, engine_layer_component*>(engine_object*)> validator;
        std::chrono::_V2::system_clock::time_point _last_time;
        float delta_time  = 0.0001f;
        float delta_time_ms  = 0.0001f;
    };

    class engine_handler {
    private:
        pl::pl_job* job = nullptr;
        pl::safe<engine_object*> m_instances = nullptr;
        pl::safe<engine_object*> m_new_instance_list = nullptr;
        pl::safe_map<std::type_index, engine_layers*> layer_data;
        pl::safe_map<std::type_index, pl::safe<engine_layer_component*>> m_layer_components;
        pl::safe_map<std::type_index, pl::safe<engine_layer_component*>> m_layer_tail_components;
        pl::safe_map<std::type_index, singleton*> m_singletons;

        size_t m_active_cores;
        std::type_index m_layer_index = std::type_index(typeid(engine_handler));
        size_t m_next_core;
        std::mutex m_layer_mtx;

        static void callback(engine_layer_component* _components);
        void process_layers(engine_object* _obj);
    public:
        [[nodiscard]] inline float get_delta_time() { return layer_data[m_layer_index]->delta_time; }
        [[nodiscard]] inline float get_delta_time_ms() { return layer_data[m_layer_index]->delta_time_ms; }

        template<typename T> inline engine_layers* get_layer() {
            return layer_data[typeid(T)];
        }

        template<typename T> inline void add_layer(const std::function<std::pair<engine_layer_component*, engine_layer_component*>(engine_object*)>& _validator) {
            auto type_index = std::type_index(typeid(T));
            layer_data.insert(std::make_pair(type_index, new engine_layers{ .validator = _validator, ._last_time = std::chrono::high_resolution_clock::now() }));
            m_layer_components.insert(std::pair(type_index, nullptr));
            m_layer_tail_components.insert(std::pair(type_index, nullptr));
        }

        template<typename T> inline T* get_or_create_singleton() {
            static_assert(std::is_base_of_v<singleton, T>, "MARS - Engine - Failed to create singleton, singleton must have mars_engine::singleton as base");
            auto type_index = std::type_index(typeid(T));

            if (m_singletons.contains(type_index))
                return (T*)m_singletons[type_index];

            auto instance = new T(this);
            m_singletons.insert(std::pair(type_index, instance));
            return instance;
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

            if (job == nullptr) {
                job = pl::async_for(0, m_active_cores, [&](int _thread_idx) {
                    callback(m_layer_components[m_layer_index].get());
                    return true;
                });
            }

            //Updating
            m_layer_components[m_layer_index].lock();
            job->start();
            job->wait();
            if (m_layer_components[m_layer_index].get() != nullptr)
                m_layer_components[m_layer_index].get()->clear();
            m_layer_components[m_layer_index].unlock();

            //Timing
            auto now = std::chrono::high_resolution_clock::now();
            layer_data[m_layer_index]->delta_time = std::chrono::duration<float, std::chrono::seconds::period>(now - layer_data[m_layer_index]->_last_time).count();
            layer_data[m_layer_index]->delta_time_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - layer_data[m_layer_index]->_last_time).count();
            layer_data[m_layer_index]->_last_time = now;
        }

        void clean();

        void spawn_wait_list();

        engine_object* instance(engine_object* _obj, mars_graphics::graphics_instance* _instance, engine_object* _parent);
        engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif