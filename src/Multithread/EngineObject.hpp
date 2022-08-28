#ifndef __ENGINE__OBJECT__
#define __ENGINE__OBJECT__

#include "ThreadManager.hpp"
#include "Graphics/VEngine.hpp"
#include "Graphics/CommandBuffer.hpp"
#include "Graphics/Material.hpp"
#include "Type/ShaderTypes.hpp"


class Transform;
template<class T> class Component;
class ComponentInterface;

class EngineObject {
private:
    CommandBuffer* commandBuffer = nullptr;
    VEngine* engine = nullptr;
public:
    vector<ComponentInterface*> components;
    vector<EngineObject*> children;

    EngineObject* parent = nullptr;
    Transform* transform  = nullptr;
    Material* material = nullptr;

    EngineObject();

    void SetEngine(VEngine* engine);
    inline VEngine* GetEngine() { return engine; }

    inline CommandBuffer* GetCommandBuffer() {
        if (commandBuffer == nullptr) {
            commandBuffer = new CommandBuffer(GetEngine());
            commandBuffer->Create();
        }

        return commandBuffer;
    }

    void ExecuteCode(ExecutionCode code);
    void AddChild(EngineObject* child);

    template<class T> T* AddComponent(T* component) {
        static_assert(is_base_of<Component<T>, T>::value, "Attempted to add a component that doesn't have Component as base");
        components.push_back(component);
        component->object = this;
        return component;
    }

    template<class T> T* AddComponent() {
        static_assert(is_base_of<Component<T>, T>::value, "Attempted to add a component that doesn't have Component as base");
        T* t = new T();
        components.push_back(t);
        t->object = this;
        return t;
    }

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