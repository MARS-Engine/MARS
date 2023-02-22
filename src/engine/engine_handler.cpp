#include <MARS/engine/engine_handler.hpp>
#include <MARS/engine/engine_object.hpp>

using namespace mars_engine;
using namespace mars_graphics;

void engine_handler::init() {
    m_active_cores = std::thread::hardware_concurrency();
}

void engine_handler::clean() {
    auto instance = m_instances.lock_get();

    while (instance != nullptr) {
        auto to_delete = instance;
        instance = to_delete->next();
        delete to_delete;
    }

    m_instances.set(nullptr);
    m_instances.unlock();

    for (auto& layer : layer_data)
            delete layer.second;

    layer_data.clear();

    for (auto& singleton : m_singletons)
        delete singleton.second;
}

void engine_handler::spawn_wait_list() {
    auto list = m_instances.lock_get();
    auto spawn_list = m_new_instance_list.lock_get();

    if (list == nullptr)
        m_instances = m_new_instance_list;
    else {
        auto tail = list->get_final();
        tail->set_next(spawn_list);
        spawn_list->set_previous(tail);
    }

    while (spawn_list != nullptr) {
        std::vector<engine_object*> stk;
        stk.push_back(spawn_list);

        while (!stk.empty()) {
            auto top = stk[stk.size() - 1];
            stk.pop_back();

            auto children = top->children().lock_get();
            while (children != nullptr) {
                stk.push_back(children);
                children = children->next();
            }
            top->children().unlock();

            process_layers(top);
        }

        spawn_list->set_engine(this);
        spawn_list = spawn_list->next();
    }

    m_new_instance_list.set(nullptr);
    m_new_instance_list.unlock();
    m_instances.unlock();
}

void engine_handler::callback(engine_layer_component* _components) {
    mars_engine::engine_layer_component* current = _components;

    while (current != nullptr) {
         if (!current->completed(true))
            current->callback(current);
        current = current->next;
    }
}

void engine_handler::process_layers(engine_object* _obj) {
    for (auto& layer : layer_data) {
        auto head_tail = layer.second->validator(_obj);

        if (head_tail.first == nullptr)
            continue;

        m_layer_tail_components[layer.first].lock();
        m_layer_components[layer.first].lock();

        if (m_layer_components[layer.first].get() == nullptr) {
            m_layer_components[layer.first].set(head_tail.first);
            if (m_layer_tail_components[layer.first].get() == nullptr)
                m_layer_tail_components[layer.first].set(head_tail.second);
        }
        else
            m_layer_tail_components[layer.first].get()->next = head_tail.first;

        m_layer_tail_components[layer.first].set(head_tail.second);

        m_layer_components[layer.first].unlock();
        m_layer_tail_components[layer.first].unlock();
    }
}

engine_object* engine_handler::instance(engine_object *_obj, graphics_instance *_instance, engine_object *_parent) {
    _obj->set_instance(_instance);

    if (_parent != nullptr) {
        _obj->set_parent(_parent);
        return _obj;
    }

    auto new_list = m_new_instance_list.lock_get();

    if (new_list == nullptr)
        m_new_instance_list = _obj;
    else {
        engine_object* prev = new_list->get_final();
        prev->set_next(_obj);
        _obj->set_previous(prev);
    }

    m_new_instance_list.unlock();

    if (++m_next_core >= m_active_cores)
        m_next_core = 0;
    return _obj;
}

engine_object* engine_handler::instance(engine_object *_obj, engine_object *_parent) {
    return instance(_obj, _parent->instance(), _parent);
}