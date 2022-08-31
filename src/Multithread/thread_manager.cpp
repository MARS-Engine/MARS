#include "thread_manager.hpp"
#include "core.hpp"
#include "engine_object.hpp"
#include <thread>
#include <algorithm>

execution_code thread_manager::curr_exec_code;
vector<core*> thread_manager::thread_cores;
vector<bool> thread_manager::completed_cores;
atomic<bool> thread_manager::threads_working;
condition_variable thread_manager::cv;
mutex thread_manager::m;
int thread_manager::prev_core_insert = 0;
vector<nanoseconds> thread_manager::_execution_time;
safe_vector<engine_object*> thread_manager::_need_destruction;

void thread_manager::init() {
    unsigned int processor_count = std::thread::hardware_concurrency();
    
    if (processor_count == 0)
        processor_count = 2; // 2 because the likelihood of a computer only having one core is very low

    thread_cores.resize(processor_count);
    completed_cores.resize(processor_count, false);
    threads_working = true;

    for (auto i = 0; i < thread_cores.size(); i++) {
        thread_cores[i] = new core();
        thread_cores[i]->init(i);
    }

    _execution_time.resize(CLEAN, {});
}

void thread_manager::execute(execution_code new_code) {
    if (new_code == LOAD)
        return debug::alert("ThreadManager - Execution code ignored, LOAD is executed only on instance");

    system_clock::time_point pre, now;
    pre = high_resolution_clock::now();
    curr_exec_code = new_code;

    for (auto i = 0; i < thread_cores.size(); i++)
        completed_cores[i] = false;

    cv.notify_all();
     unique_lock lk(m);
    cv.wait(lk, [&] { return  all_of(completed_cores.begin(), completed_cores.end(), [](bool v) { return v; }); });
    now = high_resolution_clock::now();


    _execution_time[new_code] = duration_cast<nanoseconds>(now - pre);

    if (!_need_destruction.empty()) {
        for (auto d : _need_destruction)
            final_destroy(d);
        _need_destruction.clear();
    }
}

nanoseconds thread_manager::get_execution_time(execution_code exec_code) {
    return _execution_time[exec_code];
}

void thread_manager::instance(engine_object* obj, vengine* engine, engine_object* parent) {
    obj->set_engine(engine);

    if (parent == nullptr) {
        prev_core_insert++;

        if (prev_core_insert >= thread_cores.size())
            prev_core_insert = 0;

        thread_cores[prev_core_insert]->objects.push_back(obj);
        obj->core_id = prev_core_insert;
    }
    else
        parent->add_child(obj);

    obj->execute_code(LOAD);
    obj->execute_code(PRE_RENDER);
}

void thread_manager::instance(engine_object* obj, engine_object* parent) {
    instance(obj, parent->get_engine(), parent);
}

void thread_manager::final_destroy(engine_object* obj) {
    if (obj->parent) {
        obj->parent->children->erase(find(obj->parent->children->begin(), obj->parent->children->end(), obj));
        return;
    }

    if (obj->core_id == -1)
        return;

    thread_cores[obj->core_id]->objects.erase(find(thread_cores[obj->core_id]->objects.begin(), thread_cores[obj->core_id]->objects.end(), obj));
    delete obj;
}

void thread_manager::destroy(engine_object* obj) {
    _need_destruction.push_back(obj);
}

void thread_manager::stop() {
    threads_working = false;
    cv.notify_all();
}