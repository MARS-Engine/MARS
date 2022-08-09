#include "EngineObject.hpp"
#include "Component.hpp"

void EngineObject::ExecuteCode(ExecutionCode code) {
    for (auto& component : components) {
        switch (code) {
            case LOAD:
                component->Load();
                break;
            case PRE_RENDER:
                component->PreRender();
                break;
            case UPDATE:
                component->Update();
                break;
            case RENDER:
                component->Render();
                break;
            case POST_RENDER:
                component->PostRender();
                break;
            case CLEAN:
                component->Clean();
                break;
        }
    }

    for (auto& child : children)
        child->ExecuteCode(code);
}

void EngineObject::AddComponent(Component* component) {
    components.push_back(component);
    component->object = this;
}
void EngineObject::AddChild(EngineObject* child) {
    children.push_back(child);
    child->parent = this;
    child->engine = engine;
}