#ifndef MARS_ENGINE_HANDLER_
#define MARS_ENGINE_HANDLER_

#include <pl/parallel.hpp>
#include <MARS/graphics/graphics_instance.hpp>
#include <vector>
#include "engine_object.hpp"
#include "component.hpp"

namespace mars_engine {


    struct engine_layers {
        pl::safe_map<size_t, pl::safe_vector<void*>> valid_components;
        std::function<std::vector<void*>(engine_object*)> validator;
        std::function<void(engine_layers*, int)> callback;
        float time_taken;
    };

    class engine_handler {
    private:
        pl::pl_job* job = nullptr;
        pl::safe_vector<engine_object*> m_workers;
        pl::safe_map<size_t, engine_layers*> layer_data;
        size_t m_active_cores;
        size_t m_layer_hash;
        size_t m_next_core;
        std::mutex m_layer_mtx;

        template<typename T> static std::vector<void*> validate_object(engine_object* _target) {
            std::vector<void*> result;
            for (auto& comp : _target->components()) {
                auto target = dynamic_cast<T*>(comp);
                if (target != nullptr)
                    result.push_back(target);
            }

            return result;
        }

        void process_layers(engine_object* _obj);
    public:

        template<typename T> inline engine_layers* get_layer() {
            return layer_data[typeid(T).hash_code()];
        }

        template<typename T> inline void add_layer(const std::function<void(engine_layers*, int)>& _callback) {
            auto val = layer_data.insert(std::make_pair(typeid(T).hash_code(), new engine_layers{ .validator = validate_object<T>, .callback = _callback }));
            for (size_t core = 0; core < m_active_cores; core++)
                val.first->second->valid_components.insert(std::make_pair(core, pl::safe_vector<void*>()));
        }

        void init();

        template<typename T> inline void process_layer() {
            m_layer_mtx.lock();
            auto start = std::chrono::high_resolution_clock::now();
            m_layer_hash = typeid(T).hash_code();
            if (job == nullptr) {
                job = pl::async_for(0, m_active_cores, [&](int _thread_idx) {
                    layer_data[m_layer_hash]->callback(layer_data[m_layer_hash], _thread_idx);
                    return true;
                });
            }
            job->start();
            job->wait();
            auto end = std::chrono::high_resolution_clock::now();
            layer_data[m_layer_hash]->time_taken = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1000000.f;
            m_layer_mtx.unlock();
        }

        void clean();

        engine_object* instance(engine_object* _obj, mars_graphics::graphics_instance* _instance, engine_object* _parent);
        engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif