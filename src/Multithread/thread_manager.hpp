#ifndef __THREAD__MANAGER__
#define __THREAD__MANAGER__

#include <atomic>
#include <vector>
#include <condition_variable>
#include <map>
#include <chrono>
#include "safe_vector.hpp"

using namespace std;
using namespace std::chrono;

class core;
class engine_object;
class vengine;

enum execution_code {
    PRE_LOAD = 0,
    LOAD = 1,
    PRE_RENDER = 2,
    UPDATE = 3,
    RENDER = 4,
    POST_RENDER = 5,
    CLEAN = 6
};

class thread_manager {
private:
    static vector<nanoseconds> _execution_time;
    static safe_vector<engine_object*> _need_destruction;

    static void final_destroy(engine_object* obj);

public:
    static execution_code curr_exec_code;
    static vector<core*> thread_cores;
    static vector<bool> completed_cores;
    static atomic<bool> threads_working;
    static condition_variable cv;
    static mutex m;
    static int prev_core_insert;

    static void init();
    static void execute(execution_code new_code);
    static engine_object* instance(engine_object* obj, vengine* engine = nullptr, engine_object* parent = nullptr);
    static engine_object* instance(engine_object* obj, engine_object* parent);
    static void destroy(engine_object* obj);
    static nanoseconds get_execution_time(execution_code exec_code);
    static void stop();
};


#endif