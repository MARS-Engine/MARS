#ifndef MVRE_ENGINE_HANDLER_
#define MVRE_ENGINE_HANDLER_

#include <pl/parallel.hpp>
#include <MVRE/graphics/graphics_instance.hpp>
#include <vector>
#include "engine_object.hpp"
#include "component.hpp"

namespace mvre_engine {


    struct engine_layers {
        pl::safe_map<int, pl::safe_vector<void*>> valid_components;
        std::function<std::vector<void*>(engine_object*)> validator;
        std::function<void(engine_layers*, int)> callback;
    };

    class engine_handler {
    private:
        int next_code;
        pl::pl_job* job = nullptr;
        pl::safe_map<int, pl::safe_vector<engine_object*>> m_workers;
        pl::safe_map<size_t, engine_layers*> layer_data;
        size_t m_current_hash;

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

        template<typename T> inline void add_layer(const std::function<void(engine_layers*, int)>& _callback) {
            auto val = layer_data.insert(std::make_pair(typeid(T).hash_code(), new engine_layers{ .validator = validate_object<T>, .callback = _callback }));
            for (size_t worker = 0; worker < m_workers.size(); worker++)
                val.first->second->valid_components.insert(std::make_pair(worker, pl::safe_vector<void*>()));
        }

        void init();

        template<typename T> inline void process_layer() {
            m_current_hash = typeid(T).hash_code();
            if (job == nullptr) {
                job = pl::async_for(0, m_workers.size(), [&](int _thread_idx) {
                    layer_data[m_current_hash]->callback(layer_data[m_current_hash], _thread_idx);
                    return true;
                });
            }
            job->start();
            job->wait();
        }

        void clean();

        engine_object* instance(engine_object* _obj, mvre_graphics::graphics_instance* _instance, engine_object* _parent);
        engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif