#ifndef __ENGINEOBJECT__
#define __ENGINEOBJECT__

#include "ThreadManager.hpp"
#include "Graphics/VEngine.hpp"

class Component;
class Transform;

class EngineObject {
private:
    vector<Component*> components;
    vector<EngineObject*> children;
public:
    EngineObject* parent = nullptr;
    VEngine* engine;
    Transform* transform;

    EngineObject();

    void ExecuteCode(ExecutionCode code);
    void AddComponent(Component* component);
    void AddChild(EngineObject* child);

    template<class T> T* GetComponent() {
        for (auto comp : components) {
            auto conv = dynamic_cast<T*>(comp);
            if (conv != nullptr)
                return conv;
        }

        return nullptr;
    }
};

#endif