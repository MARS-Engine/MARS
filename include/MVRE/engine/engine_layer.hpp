#ifndef __MVRE__ENGINE__LAYER__
#define __MVRE__ENGINE__LAYER__

#include "engine_handler.hpp"
#include "engine_object.hpp"

namespace mvre_engine {

    class engine_layer {
    public:
        void on_process(const pl::safe_vector<engine_object*>& _objects);

        virtual void process_engine_list(const std::vector<engine_object*>& _process_order) {  }

        template<typename T> std::vector<T*> get_valid_components(const std::vector<engine_object*>& _process_order) {
            std::vector<T*> valid_components;
            for (auto& obj : _process_order) {
                for (auto& comp : obj->components()) {
                    auto target = dynamic_cast<T*>(comp);
                    if (target != nullptr)
                        valid_components.push_back(target);
                }
            }
            return valid_components;
        }
    };
}

#endif