#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>
#include <MARS/engine/engine_worker.hpp>

using namespace mars_layers;

void mars_layers::load_layer_callback(mars_engine::layer_component_param&& _param) {
    auto start = _param.layers->begin() + _param.being;
    auto end = _param.layers->end();
    auto target = start + _param.length;
    for (auto ptr = start; ptr < target && ptr < end; ptr++)
        static_cast<load_layer*>(ptr->target)->load();
}

void mars_layers::update_layer_callback(mars_engine::layer_component_param&& _param) {
    auto start = _param.layers->begin() + _param.being;
    auto end = _param.layers->end();
    auto target = start + _param.length;
    for (auto ptr = start; ptr < target && ptr < end; ptr++)
        static_cast<update_layer*>(ptr->target)->update(*_param.layer_tick);
}

void mars_layers::post_update_layer_callback(mars_engine::layer_component_param&& _param) {
    auto start = _param.layers->begin() + _param.being;
    auto end = _param.layers->end();
    auto target = start + _param.length;
    for (auto ptr = start; ptr < target && ptr < end; ptr++)
        static_cast<post_update_layer*>(ptr->target)->post_update();
}

void mars_layers::update_gpu_callback(mars_engine::layer_component_param&& _param) {
    auto start = _param.layers->begin() + _param.being;
    auto end = _param.layers->end();
    auto target = start + _param.length;
    for (auto ptr = start; ptr < target && ptr < end; ptr++)
        static_cast<update_gpu*>(ptr->target)->send_to_gpu();
}

void mars_layers::post_render_layer_callback(mars_engine::layer_component_param&& _param) {
    auto start = _param.layers->begin() + _param.being;
    auto end = _param.layers->end();
    auto target = start + _param.length;
    for (auto ptr = start; ptr < target && ptr < end; ptr++)
        static_cast<post_render_layer*>(ptr->target)->post_render();
}