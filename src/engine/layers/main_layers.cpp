#include <MVRE/engine/layers/main_layers.hpp>
#include <MVRE/engine/component.hpp>

using namespace mvre_layers;

void mvre_layers::load_layer_callback(mvre_engine::engine_layers* _layer, int _thread) {
    for (auto component : _layer->valid_components[_thread])
        ((load_layer*)component)->load();
}

void mvre_layers::update_layer_callback(mvre_engine::engine_layers* _layer, int _thread) {
    for (auto& component : _layer->valid_components[_thread])
        ((update_layer*)component)->pre_update();
    for (auto& component : _layer->valid_components[_thread])
        ((update_layer*)component)->update();
    for (auto& component : _layer->valid_components[_thread])
        ((update_layer*)component)->post_update();
}

void mvre_layers::render_layer_callback(mvre_engine::engine_layers* _layer, int _thread) {
    for (auto& component : _layer->valid_components[_thread])
        ((render_layer*)component)->pre_render();
    for (auto& component : _layer->valid_components[_thread])
        mvre_executioner::executioner::add_job(mvre_executioner::EXECUTIONER_JOB_PRIORITY_NORMAL, ((render_layer*)component)->render_job);
    for (auto& component : _layer->valid_components[_thread])
        ((render_layer*)component)->post_render();
}