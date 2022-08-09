#include "Core.hpp"
#include "ThreadManager.hpp"
#include "EngineObject.hpp"

void Core::Init(int Index) {
    t = thread(Task, Index);
}

void Core::Task(int index) {


    while (ThreadManager::working) {
        unique_lock<decltype(ThreadManager::m)> l(ThreadManager::m);
        ThreadManager::cv.wait(l, [&]{ return !ThreadManager::completeCore[index]; });
        for (auto child : ThreadManager::coreObjects[index])
            child->ExecuteCode(ThreadManager::code);


        ThreadManager::completeCore[index] = true;
        
        l.unlock();
        ThreadManager::cv.notify_all();
    }
    
}