#include <MARS/engine/object_engine.hpp>
#include <MARS/engine/engine_worker.hpp>
#include <MARS/engine/mars_object.hpp>

using namespace mars_engine;

std::shared_ptr<engine_worker> object_engine::create_worker(size_t _cores) {
    m_workers.emplace_back(std::make_shared<engine_worker>(mars_ref<object_engine>(get_ptr()), _cores));
    return m_workers.back()->get_ptr();
}

mars_ref<mars_object> object_engine::create_obj() {
    auto obj  = std::make_shared<mars_object>(mars_ref<object_engine>(shared_from_this()));
    m_objects.push_back(obj);
    return mars_ref<mars_object>(obj);
}

void object_engine::spawn_wait_list() {
    for (auto& _obj : m_destroy_list) {
        for (auto& layer : m_layer_data) {
            m_layer_calls.at(layer.first)->erase(std::remove_if(m_layer_calls.at(layer.first)->begin(), m_layer_calls.at(layer.first)->end(), [&](const engine_layer_component& val) {
                return val.parent == _obj.get();
            }), m_layer_calls.at(layer.first)->end());

            m_wait_list.at(layer.first)->erase(std::remove_if(m_wait_list.at(layer.first)->begin(), m_wait_list.at(layer.first)->end(), [&](const engine_layer_component& val) {
                return val.parent == _obj.get();
            }), m_wait_list.at(layer.first)->end());
        }

        m_objects.erase(std::find(m_objects.begin(), m_objects.end(), _obj));
    }

    m_destroy_list.clear();

    for (auto& layer : m_layer_data) {
        if (m_wait_list.at(layer.first)->empty())
            continue;

        m_layer_calls.at(layer.first)->reserve(m_layer_calls.at(layer.first)->size() + m_wait_list.at(layer.first)->size());
        m_layer_calls.at(layer.first)->insert(m_layer_calls.at(layer.first)->end(), m_wait_list.at(layer.first)->begin(), m_wait_list.at(layer.first)->end());
        m_wait_list.at(layer.first)->clear();
    }
}

void object_engine::process_component(const mars_ref<mars_engine::component>& _component) {
    for (auto& layer : m_layer_data) {
        engine_layer_component val;
        if (m_layer_data.at(layer.first).m_validator(_component, val))
            m_wait_list.at(layer.first)->push_back(val);
    }
    layers_waiting = true;
}

mars_ref<mars_object> object_engine::spawn(const mars_ref<mars_object>& _obj, const mars_ref<mars_graphics::graphics_engine>& _graphics, const mars_ref<mars_object>& _parent) {
    _obj->set_graphics(_graphics);

    m_objects.lock();
    m_objects.push_back(_obj.get().lock());
    m_objects.unlock();

    return _obj;
}

void object_engine::clean() {
    for (auto& object : m_objects)
        object->destroy();
}