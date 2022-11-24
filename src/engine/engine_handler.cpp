#include "MVRE/engine/engine_handler.hpp"
#include "MVRE/engine/engine_worker.hpp"

using namespace mvre_engine;
using namespace mvre_graphics;

int engine_handler::next_code = 0;
std::vector<engine_worker*> engine_handler::workers;
pl::pl_job* engine_handler::m_job = nullptr;

void engine_handler::init() {
    workers.resize(std::thread::hardware_concurrency());
    for (auto& worker : workers)
        worker = new engine_worker();
}

void engine_handler::execute(MVRE_EXECUTION_CODE _code) {
    if (m_job == nullptr) {
        m_job = pl::async_foreach<engine_worker*>(workers.data(), workers.size(), [&](engine_worker* worker) {
            worker->execute(_code);
            return true;
        });
    }
    m_job->start();
    m_job->wait();
}

void engine_handler::clean() {
    m_job->clean();
    delete m_job;

    for (auto worker : workers)
        delete worker;

    workers.clear();
    workers.shrink_to_fit();
}

engine_object* engine_handler::instance(engine_object *_obj, graphics_instance *_instance, engine_object *_parent) {
    _obj->set_instance(_instance);
    _obj->set_parent(_parent);
    workers[next_code++]->objects.push_back(_obj);
    return _obj;
}

engine_object* engine_handler::instance(engine_object *_obj, engine_object *_parent) {
    return instance(_obj, _parent->instance(), _parent);
}