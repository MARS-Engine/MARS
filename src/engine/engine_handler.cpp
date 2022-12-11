#include <MVRE/engine/engine_handler.hpp>
#include <MVRE/engine/engine_object.hpp>

using namespace mvre_engine;
using namespace mvre_graphics;

void engine_handler::init() {
    for (auto i = 0; i < std::thread::hardware_concurrency(); i++)
        m_workers[i] = pl::safe_vector<engine_object*>();
}

void engine_handler::clean() {

    m_workers.clear();

    for (auto& layer : layer_data)
            delete layer.second;
    layer_data.clear();
}

engine_object* engine_handler::instance(engine_object *_obj, graphics_instance *_instance, engine_object *_parent) {
    _obj->set_instance(_instance);

    for (auto& layer : layer_data) {
        auto valid_layers = layer.second->validator(_obj);
        for (auto component : valid_layers)
            layer.second->valid_components[next_code].push_back(component);
    }

    if (_parent != nullptr) {
        _obj->set_parent(_parent);
        return _obj;
    }

    m_workers[next_code].push_back(_obj);
    if (++next_code >= m_workers.size())
        next_code = 0;
    return _obj;
}

engine_object* engine_handler::instance(engine_object *_obj, engine_object *_parent) {
    return instance(_obj, _parent->instance(), _parent);
}