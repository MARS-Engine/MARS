#include <MVRE/engine/engine_layer.hpp>
#include <MVRE/engine/engine_object.hpp>

using namespace mvre_engine;

void engine_layer::on_process(const pl::safe_vector<mvre_engine::engine_object*>& _objects) {
    std::vector<mvre_engine::engine_object*> stack;
    std::vector<mvre_engine::engine_object*> process_list;

    for (auto& obj : _objects) {
        stack.clear();
        stack.shrink_to_fit();
        stack.push_back(obj);

        while (!stack.empty()) {
            mvre_engine::engine_object* top = stack[stack.size() - 1];
            stack.pop_back();

            for (auto& child : top->children())
                stack.push_back(child);

            process_list.push_back(top);
        }
    }

    process_engine_list(process_list);
}