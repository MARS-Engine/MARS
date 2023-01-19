#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>

using namespace mars_layers;

void mars_layers::load_layer_callback(mars_engine::engine_layers* _layer, int _thread) {
    for (auto component : _layer->valid_components[_thread])
        ((load_layer*)component)->load();
}

void mars_layers::update_layer_callback(mars_engine::engine_layers* _layer, int _thread) {
    for (auto& component : _layer->valid_components[_thread])
        ((update_layer*)component)->pre_update();
    for (auto& component : _layer->valid_components[_thread])
        ((update_layer*)component)->update();
    for (auto& component : _layer->valid_components[_thread])
        ((update_layer*)component)->post_update();
}

void mars_layers::render_update_layer_callback(mars_engine::engine_layers* _layer, int _thread) {
    for (auto& component : _layer->valid_components[_thread])
        ((render_update_layer*)component)->prepare_gpu();
}

void mars_layers::render_layer_callback(mars_engine::engine_layers* _layer, int _thread) {
    for (auto& component : _layer->valid_components[_thread])
        ((render_layer*)component)->send_to_gpu();
    for (auto& component : _layer->valid_components[_thread])
        ((render_layer*)component)->pre_render();
    for (auto& component : _layer->valid_components[_thread])
        if (((render_layer*)component)->render_job != nullptr)
            mars_executioner::executioner::add_job(mars_executioner::EXECUTIONER_JOB_PRIORITY_NORMAL, ((render_layer*)component)->render_job);
    for (auto& component : _layer->valid_components[_thread])
        ((render_layer*)component)->post_render();
}