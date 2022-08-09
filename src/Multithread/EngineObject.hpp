#ifndef __ENGINEOBJECT__
#define __ENGINEOBJECT__

#include "ThreadManager.hpp"
#include "Graphics/VEngine.hpp"

class Component;

class EngineObject {
private:
    vector<Component*> components;
    vector<EngineObject*> children;
public:
    EngineObject* parent;
    VEngine* engine;

    void ExecuteCode(ExecutionCode code);
    void AddComponent(Component* component);
    void AddChild(EngineObject* child);
};

#endif