#include "ThreadManager.hpp"
#include "Core.hpp"
#include "EngineObject.hpp"
#include <thread>
#include <algorithm>

ExecutionCode ThreadManager::code;
vector<Core*> ThreadManager::cores;
vector<bool> ThreadManager::completeCore;
atomic<bool> ThreadManager::working;
map<unsigned int, vector<EngineObject*>> ThreadManager::coreObjects;
condition_variable ThreadManager::cv;
mutex ThreadManager::m;
int ThreadManager::previousCoreInsert = 0;
vector<nanoseconds> ThreadManager::ExecutionTime;

void ThreadManager::Init() {
    unsigned int processor_count = std::thread::hardware_concurrency();
    
    if (processor_count == 0)
        processor_count = 1;

    cores.resize(processor_count);
    completeCore.resize(processor_count, false);
    working = true;

    for (auto i = 0; i < cores.size(); i++) {
        cores[i] = new Core();
        cores[i]->Init(i);
    }

    ExecutionTime.resize(CLEAN, {});
}

void ThreadManager::Execute(ExecutionCode newCode) {
    if (newCode == LOAD)
        return Debug::Alert("ThreadManager - Execution code ignored, LOAD is executed only on instance");

    system_clock::time_point Pre, Now;
    Pre = high_resolution_clock::now();
    code = newCode;

    for (auto i = 0; i < cores.size(); i++)
        completeCore[i] = false;

    cv.notify_all();
     unique_lock lk(m);
    cv.wait(lk, [&] { return  all_of(completeCore.begin(), completeCore.end(), [](bool v) { return v; }); });
    Now = high_resolution_clock::now();

    ExecutionTime[newCode] = duration_cast<nanoseconds>(Now - Pre);
}

nanoseconds ThreadManager::GetExecutionTime(ExecutionCode _code) {
    return ExecutionTime[_code];
}

void ThreadManager::Instance(EngineObject* obj, VEngine* engine, EngineObject* parent) {
    obj->SetEngine(engine);

    if (parent == nullptr) {
        previousCoreInsert++;

        if (previousCoreInsert >= coreObjects.size())
            previousCoreInsert = 0;

        coreObjects[previousCoreInsert].push_back(obj);
    }
    else
        parent->AddChild(obj);

    obj->ExecuteCode(LOAD);
}

void ThreadManager::Instance(EngineObject* obj, EngineObject* parent) {
    Instance(obj, parent->GetEngine(), parent);
}

void ThreadManager::Stop() {
    working = false;
    cv.notify_all();
}