#ifndef __THREAD__MANAGER__
#define __THREAD__MANAGER__

#include <atomic>
#include <vector>
#include <condition_variable>
#include <map>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Core;
class EngineObject;
class VEngine;

enum ExecutionCode {
    PRE_LOAD = 0,
    LOAD = 1,
    PRE_RENDER = 2,
    UPDATE = 3,
    RENDER = 4,
    POST_RENDER = 5,
    CLEAN = 6
};

class ThreadManager {
private:
    static vector<nanoseconds> ExecutionTime;
public:
    static ExecutionCode code;
    static vector<Core*> cores;
    static vector<bool> completeCore;
    static atomic<bool> working;
    static map<unsigned int, vector<EngineObject*>> coreObjects;
    static condition_variable cv;
    static mutex m;
    static int previousCoreInsert;

    static void Init();
    static void Execute(ExecutionCode newCode);
    static void Instance(EngineObject* obj, VEngine* engine = nullptr, EngineObject* parent = nullptr);
    static void Instance(EngineObject* obj, EngineObject* parent);
    static void Destroy(EngineObject* obj);
    static nanoseconds GetExecutionTime(ExecutionCode code);
    static void Stop();
};


#endif