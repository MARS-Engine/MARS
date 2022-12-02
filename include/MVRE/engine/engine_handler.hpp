#ifndef __MVRE__ENGINE__HANDLER__
#define __MVRE__ENGINE__HANDLER__

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
        pl::pl_job* m_job;
        pl::safe_map<int, pl::safe_vector<engine_object*>> m_workers;
        pl::safe_map<size_t, engine_layers*> layer_data;

        template<typename T> static std::vector<void*> validate_object(engine_object* _target) {
            std::vector<void*> result;
            for (auto& comp : _target->components()) {
                auto target = dynamic_cast<T*>(comp);
                if (target != nullptr)
                    result.push_back(target);
            }

            return result;
        }

    public:
        pl::safe_map<int, pl::safe_vector<engine_object*>>& workers() { return m_workers; };

        template<typename T> inline void add_layer(std::function<void(engine_layers*, int)> _callback) {
            layer_data.insert(typeid(T).hash_code(), new engine_layers{ .validator = validate_object<T>, .callback = _callback });
        }

        void init();

        template<typename T> void process_layer() {
            auto hash __attribute__((unused)) = typeid(T).hash_code();
            if (m_job == nullptr) {
                m_job = pl::async_for(0, m_workers.size(), [&](int _thread_idx) {
                    layer_data[hash]->callback(layer_data[hash], _thread_idx);
                    return true;
                });
            }
            m_job->start();
            m_job->wait();
        }

        void clean();

        engine_object* instance(engine_object* _obj, mvre_graphics::graphics_instance* _instance, engine_object* _parent);
        engine_object* instance(engine_object* _obj, engine_object* _parent);
    };
}

#endif