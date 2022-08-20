#ifndef __THREADMANAGER__
#define __THREADMANAGER__

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
    PRE_LOAD,
    LOAD,
    PRE_RENDER,
    UPDATE,
    RENDER,
    POST_RENDER,
    CLEAN
};

class ThreadManager {
public:
    static ExecutionCode code;
    static vector<Core*> cores;
    static vector<bool> completeCore;
    static atomic<bool> working;
    static map<unsigned int, vector<EngineObject*>> coreObjects;
    static condition_variable cv;
    static mutex m;
    static int previousCoreInsert;
    static nanoseconds UpdateTime;
    static nanoseconds RenderTime;
    static nanoseconds PosRenderTime;

    static void Init();
    static void Execute(ExecutionCode newCode);
    static void Instance(EngineObject* obj, VEngine* engine = nullptr, EngineObject* parent = nullptr);
    static void Instance(EngineObject* obj, EngineObject* parent);
    static void Stop();
};


#endif