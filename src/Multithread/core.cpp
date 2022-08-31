#include "core.hpp"
#include "thread_manager.hpp"
#include "engine_object.hpp"

void core::init(int index) {
    _index = index;
    _t = thread(&core::work, this);
}

void core::work() {
    while (thread_manager::threads_working) {
        unique_lock<decltype(thread_manager::m)> l(thread_manager::m);
        thread_manager::cv.wait(l, [&]{ return !thread_manager::completed_cores[_index] || !thread_manager::threads_working; });
        for (auto child : objects)
            child->execute_code(thread_manager::curr_exec_code);
        thread_manager::completed_cores[_index] = true;
        l.unlock();
        thread_manager::cv.notify_all();
    }
    
}