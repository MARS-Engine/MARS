#include <MARS/engine/engine_handler.hpp>
#include <MARS/engine/engine_object.hpp>

using namespace mars_engine;
using namespace mars_graphics;

void engine_handler::init() {
    m_active_cores = std::thread::hardware_concurrency();
}

void engine_handler::clean() {
    for (auto& i : m_workers)
        delete i;
    m_workers.clear();

    for (auto& layer : layer_data)
            delete layer.second;

    layer_data.clear();
}

void engine_handler::process_layers(engine_object* _obj) {
    for (auto& layer : layer_data) {
        auto valid_layers = layer.second->validator(_obj);
        layer.second->valid_components[m_next_core].lock();
        for (auto component: valid_layers)
            layer.second->valid_components[m_next_core].push_back(component);
        layer.second->valid_components[m_next_core].unlock();
    }
}

engine_object* engine_handler::instance(engine_object *_obj, graphics_instance *_instance, engine_object *_parent) {
    _obj->set_instance(_instance);

    std::vector<engine_object*> stk;
    stk.push_back(_obj);

    while (!stk.empty()) {
        auto top = stk[stk.size() - 1];
        stk.pop_back();

        for (auto child : top->children())
            stk.push_back(child);

        process_layers(top);
    }

    if (_parent != nullptr) {
        _obj->set_parent(_parent);
        return _obj;
    }

    m_workers.lock();
    m_workers.push_back(_obj);
    m_workers.unlock();

    if (++m_next_core >= m_active_cores)
        m_next_core = 0;
    return _obj;
}

engine_object* engine_handler::instance(engine_object *_obj, engine_object *_parent) {
    return instance(_obj, _parent->instance(), _parent);
}