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
nanoseconds ThreadManager::UpdateTime;
nanoseconds ThreadManager::RenderTime;
nanoseconds ThreadManager::PosRenderTime;

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
}

void ThreadManager::Execute(ExecutionCode newCode) {
    system_clock::time_point Pre, Now;
    Pre = high_resolution_clock::now();
    code = newCode;

    for (auto i = 0; i < cores.size(); i++)
        completeCore[i] = false;

    cv.notify_all();
     unique_lock lk(m);
    cv.wait(lk, [&] { return  all_of(completeCore.begin(), completeCore.end(), [](bool v) { return v; }); });
    Now = high_resolution_clock::now();

    switch (newCode) {
        case UPDATE:
            UpdateTime = duration_cast<nanoseconds>(Now - Pre);
            break;
        case RENDER:
            RenderTime = duration_cast<nanoseconds>(Now - Pre);
            break;
        case POST_RENDER:
            PosRenderTime = duration_cast<nanoseconds>(Now - Pre);
            break;
    }
}

void ThreadManager::Instance(EngineObject* obj, VEngine* engine, EngineObject* parent) {
    previousCoreInsert++;

    if (previousCoreInsert >= coreObjects.size())
        previousCoreInsert = 0;

    obj->SetEngine(engine);

    if (parent == nullptr)
        coreObjects[previousCoreInsert].push_back(obj);
    else
        parent->AddChild(obj);
}

void ThreadManager::Instance(EngineObject* obj, EngineObject* parent) {
    Instance(obj, parent->GetEngine(), parent);
}