#include "MVRE/engine/engine_handler.hpp"
#include "MVRE/engine/engine_worker.hpp"

#include <pl/parallel.hpp>

using namespace mvre_engine;
using namespace mvre_graphics;
using namespace pl;

int engine_handler::next_code = 0;
std::vector<engine_worker*> engine_handler::workers;

void engine_handler::init() {
    parallel::init(PARALLEL_CORES_ALL_MINUS_ONE);
    workers.resize(parallel::n_threads);
    for (auto& worker : workers)
        worker = new engine_worker();
}

void engine_handler::execute(MVRE_EXECUTION_CODE _code) {
    parallel::_foreach<engine_worker*>(workers.data(), workers.size(), [&](engine_worker* worker) {
        worker->execute(_code);
        return true;
    });
}

void engine_handler::clean() {
    for (auto worker : workers)
        delete worker;

    workers.clear();
    workers.shrink_to_fit();
}

engine_object* engine_handler::instance(engine_object *_obj, engine_instance *_instance, engine_object *_parent) {
    _obj->set_instance(_instance);
    _obj->set_parent(_parent);
    workers[next_code++]->objects.push_back(_obj);
    return _obj;
}

engine_object* engine_handler::instance(engine_object *_obj, engine_object *_parent) {
    return instance(_obj, _parent->instance(), _parent);
}