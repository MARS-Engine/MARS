#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/engine_worker.hpp>

using namespace mars_layers;

void mars_layers::load_layer_callback(mars_engine::layer_component_param&& _param) {
    for (auto ptr = 0; ptr < _param.length && ptr + _param.being < _param.layers->size(); ptr++)
        static_cast<load_layer*>(_param.layers->at(ptr + _param.being).target)->load();
}

void mars_layers::update_layer_callback(mars_engine::layer_component_param&& _param) {
    for (auto ptr = 0; ptr < _param.length && ptr + _param.being < _param.layers->size(); ptr++)
        static_cast<update_layer*>(_param.layers->at(ptr + _param.being).target)->update(*_param.layer_tick);
}

void mars_layers::post_update_layer_callback(mars_engine::layer_component_param&& _param) {
    for (auto ptr = 0; ptr < _param.length && ptr + _param.being < _param.layers->size(); ptr++)
        static_cast<post_update_layer*>(_param.layers->at(ptr + _param.being).target)->post_update();
}

void mars_layers::update_gpu_callback(mars_engine::layer_component_param&& _param) {
    for (auto ptr = 0; ptr < _param.length && ptr + _param.being < _param.layers->size(); ptr++)
        static_cast<update_gpu*>(_param.layers->at(ptr + _param.being).target)->send_to_gpu();
}

void mars_layers::post_render_layer_callback(mars_engine::layer_component_param&& _param) {
    for (auto ptr = 0; ptr < _param.length && ptr + _param.being < _param.layers->size(); ptr++)
        static_cast<post_render_layer*>(_param.layers->at(ptr + _param.being).target)->post_render();
}