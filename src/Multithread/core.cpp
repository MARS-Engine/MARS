#include "core.hpp"
#include "thread_manager.hpp"
#include "engine_object.hpp"

void core::init(int index) {
    _t = thread(work, index);
}

void core::work(int index) {
    while (thread_manager::threads_working) {
        unique_lock<decltype(thread_manager::m)> l(thread_manager::m);
        thread_manager::cv.wait(l, [&]{ return !thread_manager::completed_cores[index] || !thread_manager::threads_working; });
        for (auto child : thread_manager::cores_objects[index])
            child->execute_code(thread_manager::curr_exec_code);
        thread_manager::completed_cores[index] = true;
        l.unlock();
        thread_manager::cv.notify_all();
    }
    
}