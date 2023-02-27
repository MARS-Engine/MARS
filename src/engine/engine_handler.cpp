#include <MARS/engine/engine_handler.hpp>
#include <MARS/engine/engine_object.hpp>

using namespace mars_engine;
using namespace mars_graphics;

void engine_handler::init() {
    m_active_cores = std::thread::hardware_concurrency();
    for (size_t i = 0; i < m_active_cores; i++)
        m_workers.push_back(new engine_worker(this));
}

void engine_handler::clean() {
    auto head = m_objects.lock_get();

    while (head != nullptr) {
        auto to_delete = head;
        head = to_delete->next();
        delete to_delete;
    }

    m_objects.set(nullptr);
    m_objects.unlock();

    for (auto& layer : layer_data)
            delete layer.second;

    layer_data.clear();

    for (auto& singleton : m_singletons)
        delete singleton.second;
}

void engine_handler::spawn_wait_list() {
    auto list = m_objects.lock_get();
    auto spawn_list = m_new_objects.lock_get();

    if (list == nullptr)
        m_objects = m_new_objects;
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

    m_new_objects.set(nullptr);
    m_new_objects.unlock();
    m_objects.unlock();
}

void engine_handler::callback(const pl::safe_vector<engine_layer_component*>& _components) {
    for (engine_layer_component* component : _components) {
        if (!component->completed(true))
            component->callback(component);
    }
}

void engine_handler::process_layers(engine_object* _obj) {
    for (auto& layer : layer_data) {
        std::vector<engine_layer_component*> list = layer.second->validator(_obj);

        if (list.empty())
            continue;

        if (!m_layer_components[layer.first].empty())
            m_layer_components[layer.first][m_layer_components[layer.first].size() - 1]->next = list[0];

        m_layer_components[layer.first].insert(m_layer_components[layer.first].end(), list.begin(), list.end());
    }
}

engine_object* engine_handler::spawn(engine_object *_obj, graphics_engine *_graphics, engine_object *_parent) {
    _obj->set_graphics(_graphics);

    if (_parent != nullptr) {
        _obj->set_parent(_parent);
        return _obj;
    }

    auto new_list = m_new_objects.lock_get();

    if (new_list == nullptr)
        m_new_objects = _obj;
    else {
        engine_object* prev = new_list->get_final();
        prev->set_next(_obj);
        _obj->set_previous(prev);
    }

    m_new_objects.unlock();

    if (++m_next_core >= m_active_cores)
        m_next_core = 0;
    return _obj;
}

engine_object* engine_handler::spawn(engine_object *_obj, engine_object *_parent) {
    return spawn(_obj, _parent->graphics(), _parent);
}