#include <MVRE/engine/engine_handler.hpp>
#include <MVRE/engine/engine_layer.hpp>
#include <MVRE/engine/engine_object.hpp>

using namespace mvre_engine;
using namespace mvre_graphics;

int engine_handler::next_code = 0;
pl::safe_map<int, pl::safe_vector<engine_object*>> engine_handler::m_workers;
pl::pl_job* engine_handler::m_job = nullptr;

void engine_handler::init() {
    for (auto i = 0; i < std::thread::hardware_concurrency(); i++)
        m_workers[i] = pl::safe_vector<engine_object*>();
}

void engine_handler::process_layer(engine_layer* _layer) {
    if (m_job == nullptr) {
        m_job = pl::async_for(0, m_workers.size(), [&](int _thread_idx) {
            _layer->on_process(m_workers[_thread_idx]);
            return true;
        });
    }
    m_job->start();
    m_job->wait();
}

void engine_handler::clean() {
    m_job->clean();
    delete m_job;

    m_workers.clear();
}

engine_object* engine_handler::instance(engine_object *_obj, graphics_instance *_instance, engine_object *_parent) {
    _obj->set_instance(_instance);
    if (_parent != nullptr)
        _obj->set_parent(_parent);
    m_workers[next_code++].push_back(_obj);
    if (next_code >= m_workers.size())
        next_code = 0;
    return _obj;
}

engine_object* engine_handler::instance(engine_object *_obj, engine_object *_parent) {
    return instance(_obj, _parent->instance(), _parent);
}