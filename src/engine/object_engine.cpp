#include <MARS/engine/object_engine.hpp>
#include <MARS/engine/engine_worker.hpp>
#include <MARS/engine/mars_object.hpp>

using namespace mars_engine;

std::shared_ptr<engine_worker> object_engine::create_worker(size_t _cores) {
    m_workers.emplace_back(std::make_shared<engine_worker>(mars_ref<object_engine>(shared_from_this()), _cores));
    return m_workers.back()->get_ptr();
}

mars_ref<mars_object> object_engine::create_obj() {
    auto obj = std::make_shared<mars_object>();
    obj->set_engine(mars_ref<object_engine>(shared_from_this()));
    m_objects.lock()->push_back(obj);
    return mars_ref<mars_object>(obj);
}

void object_engine::spawn_wait_list() {
    auto layer_data = m_layer_data.lock();
    auto destroy_list = m_destroy_list.lock();
    for (auto& _obj : *destroy_list.get()) {
        for (auto& layer : *layer_data.get()) {
            std::vector<engine_layer_component>& call_layer = *m_layer_calls.at(layer.first);
            std::vector<engine_layer_component>& wait_layer = *m_wait_list.at(layer.first);

            call_layer.erase(std::remove_if(call_layer.begin(), call_layer.end(), [&](const engine_layer_component& val) {
                return val.parent == _obj.get();
            }), call_layer.end());

            wait_layer.erase(std::remove_if(wait_layer.begin(), wait_layer.end(), [&](const engine_layer_component& val) {
                return val.parent == _obj.get();
            }), wait_layer.end());
        }

        auto objects = m_objects.lock();
        objects->erase(std::find(objects->begin(), objects->end(), _obj));
    }

    if (!destroy_list->empty())
        destroy_list->clear();

    for (auto& layer : *layer_data.get()) {
        if (m_wait_list.at(layer.first)->empty())
            continue;

        std::vector<engine_layer_component>& call_layer = *m_layer_calls.at(layer.first);
        std::vector<engine_layer_component>& wait_layer = *m_wait_list.at(layer.first);

        call_layer.reserve(call_layer.size() + wait_layer.size());
        call_layer.insert(call_layer.end(), wait_layer.begin(), wait_layer.end());
        wait_layer.clear();
    }
}

void object_engine::process_component(const mars_ref<mars_engine::component>& _component) {
    for (auto& layer : *m_layer_data.lock().get()) {
        engine_layer_component val;
        if (layer.second.m_validator(_component, val))
            m_wait_list.at(layer.first)->push_back(val);
    }
    layers_waiting = true;
}

mars_ref<mars_object> object_engine::spawn(const mars_ref<mars_object>& _obj, const mars_ref<mars_graphics::graphics_engine>& _graphics, const mars_ref<mars_object>& _parent) {
    m_objects.lock()->push_back(_obj.get().lock());
    return _obj;
}

void object_engine::clean() {
    m_objects.lock()->clear();
    m_wait_list.clear();
    m_layer_calls.clear();
    m_layer_data.lock()->clear();
    m_singletons.lock()->clear();
}