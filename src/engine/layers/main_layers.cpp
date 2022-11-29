#include <MVRE/engine/layers/main_layers.hpp>
#include <MVRE/engine/component.hpp>

using namespace mvre_layers;

void load_layer::process_engine_list(const std::vector<mvre_engine::engine_object*>& _process_order) {
    for (auto& component : get_valid_components<component_main_layer>(_process_order))
        component->load();
}

void update_layer::process_engine_list(const std::vector<mvre_engine::engine_object*>& _process_order) {
    auto components = get_valid_components<component_main_layer>(_process_order);

    for (auto& component : components)
        component->pre_update();
    for (auto& component : components)
        component->update();
    for (auto& component : components)
        component->post_update();
}

void render_layer::process_engine_list(const std::vector<mvre_engine::engine_object*>& _process_order) {
    auto components = get_valid_components<component_main_layer>(_process_order);

    for (auto& component : components)
        component->pre_render();
    for (auto& object : _process_order)
        for (auto& component : object->components())
            if (component->render_job != nullptr)
                mvre_executioner::executioner::add_job(mvre_executioner::EXECUTIONER_JOB_PRIORITY_NORMAL, component->render_job);
    for (auto& component : components)
        component->post_render();
}