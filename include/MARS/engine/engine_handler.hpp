#ifndef MARS_ENGINE_HANDLER_
#define MARS_ENGINE_HANDLER_

#include <pl/parallel.hpp>
#include <pl/safe.hpp>
#include <MARS/graphics/graphics_instance.hpp>
#include <vector>
#include "engine_object.hpp"
#include "component.hpp"

namespace mars_engine {

    struct engine_layer_component {
        std::atomic<bool> completed = false;

        engine_layer_component* next;
        engine_layer_component* previous;
        engine_object* parent;

        std::function<void(void)> callback;

        void clear() {
            completed = false;
            auto next_clear = next;

            while (next_clear != nullptr) {
                next_clear->completed = false;
                next_clear = next_clear->next;
            }
        }
    };

    struct engine_layers {
        std::function<std::vector<std::function<void(void)>>(engine_object*)> validator;
        std::chrono::_V2::system_clock::time_point _last_time;
        float delta_time  = 0.0001f;
        float delta_time_ms  = 0.0001f;
    };

    class engine_handler {
    private:
        pl::pl_job* job = nullptr;
        pl::safe<engine_object*> m_instances = nullptr;
        pl::safe<engine_object*> m_new_instance_list = nullptr;
        pl::safe_map<size_t, engine_layers*> layer_data;
        pl::safe_map<size_t, pl::safe<engine_layer_component*>> m_layer_components;
        pl::safe_map<size_t, pl::safe<engine_layer_component*>> m_layer_tail_components;

        size_t m_active_cores;
        size_t m_layer_hash;
        size_t m_next_core;
        std::mutex m_layer_mtx;

        static void callback(engine_layer_component* _components);
        void process_layers(engine_object* _obj);
    public:
        [[nodiscard]] inline float get_delta_time() { return layer_data[m_layer_hash]->delta_time; }
        [[nodiscard]] inline float get_delta_time_ms() { return layer_data[m_layer_hash]->delta_time_ms; }

        template<typename T> inline engine_layers* get_layer() {
            return layer_data[typeid(T).hash_code()];
        }

        template<typename T> inline void add_layer(const std::function<std::vector<std::function<void(void)>>(engine_object*)>& _validator) {
            auto hash = typeid(T).hash_code();
            layer_data.insert(std::make_pair(hash, new engine_layers{ .validator = _validator, ._last_time = std::chrono::high_resolution_clock::now() }));
            m_layer_components.insert(std::pair(hash, nullptr));
            m_layer_tail_components.insert(std::pair(hash, nullptr));
        }

        void init();

        inline void lock() {
            m_layer_mtx.lock();
        }

        inline void unlock() {
            m_layer_mtx.unlock();
        }

        template<typename T> inline void process_layer() {
            m_layer_hash = typeid(T).hash_code();

            if (job == nullptr) {
                job = pl::async_for(0, m_active_cores, [&](int _thread_idx) {
                    callback(m_layer_components[m_layer_hash].get());
                    return true;
                });
            }

            //Updating
            m_layer_components[m_layer_hash].lock();
            job->start();
            job->wait();
            if (m_layer_components[m_layer_hash].get() != nullptr)
                m_layer_components[m_layer_hash].get()->clear();
            m_layer_components[m_layer_hash].unlock();

            //Timing
            auto now = std::chrono::high_resolution_clock::now();
            layer_data[m_layer_hash]->delta_time = std::chrono::duration<float, std::chrono::seconds::period>(now - layer_data[m_layer_hash]->_last_time).count();
            layer_data[m_layer_hash]->delta_time_ms = std::chrono::duration<float, std::chrono::milliseconds::period>(now - layer_data[m_layer_hash]->_last_time).count();
            layer_data[m_layer_hash]->_last_time = now;
        }

        void clean();

        void spawn_wait_list();

        engine_object* instance(engine_object* _obj, mars_graphics::graphics_instance* _instance, engine_object* _parent);
        engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif