#include <MARS/engine/layers/main_layers.hpp>
#include <MARS/engine/component.hpp>
#include <MARS/engine/engine_worker.hpp>

using namespace mars_layers;

void mars_layers::load_layer_callback(const mars_engine::layer_component_param& _param) {
    std::atomic<size_t>& index = _param._worker->get_index();
    for (size_t i = index.fetch_add(1); i < _param.layers->size(); i = index.fetch_add(1))
        static_cast<load_layer*>(_param.layers->at(i).target)->load();
}

void mars_layers::update_layer_callback(const mars_engine::layer_component_param& _param) {
    std::atomic<size_t>& index = _param._worker->get_index();
    for (size_t i = index.fetch_add(1); i < _param.layers->size(); i = index.fetch_add(1))
        static_cast<update_layer*>(_param.layers->at(i).target)->update(*_param.layer_tick);
}

void mars_layers::post_update_layer_callback(const mars_engine::layer_component_param& _param) {
    std::atomic<size_t>& index = _param._worker->get_index();
    for (size_t i = index.fetch_add(1); i < _param.layers->size(); i = index.fetch_add(1))
        static_cast<post_update_layer*>(_param.layers->at(i).target)->post_update();
}

void mars_layers::update_gpu_callback(const mars_engine::layer_component_param& _param) {
    std::atomic<size_t>& index = _param._worker->get_index();
    for (size_t i = index.fetch_add(1); i < _param.layers->size(); i = index.fetch_add(1))
        static_cast<update_gpu*>(_param.layers->at(i).target)->send_to_gpu();
}

void mars_layers::post_render_layer_callback(const mars_engine::layer_component_param& _param) {
    std::atomic<size_t>& index = _param._worker->get_index();
    for (size_t i = index.fetch_add(1); i < _param.layers->size(); i = index.fetch_add(1))
        static_cast<post_render_layer*>(_param.layers->at(i).target)->post_render();
}