#include <MARS/engine/object_engine.hpp>
#include <MARS/engine/engine_worker.hpp>

using namespace mars_engine;

std::shared_ptr<engine_worker> _object_engine::create_worker(size_t _cores) {
    m_workers.emplace_back(std::make_shared<engine_worker>(get_ptr(), _cores));
    return m_workers.back()->get_ptr();
}

void _object_engine::spawn_wait_list() {
    m_new_objects.lock();

    for (auto& object : m_new_objects) {
        m_objects.lock();
        m_objects.push_back(object);
        m_objects.unlock();
        process_layers(object);
    }

    m_new_objects.unlock();
}

void _object_engine::process_layers(const mars_object& _obj) {
    for (auto& layer : m_layer_data) {
        std::vector<engine_layer_component> list = layer.second.m_validator(_obj);

        if (list.empty())
            continue;

        m_layer_calls[layer.first]->insert(m_layer_calls[layer.first]->end(), list.begin(), list.end());
    }
}

mars_object _object_engine::spawn(const mars_object& _obj, const mars_graphics::graphics_engine& _graphics, const mars_object& _parent) {
    _obj->set_graphics(_graphics);

    m_new_objects.lock();
    m_new_objects.push_back(_obj);
    m_new_objects.unlock();

    return _obj;
}