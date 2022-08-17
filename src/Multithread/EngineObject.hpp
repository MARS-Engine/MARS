#ifndef __ENGINEOBJECT__
#define __ENGINEOBJECT__

#include "ThreadManager.hpp"
#include "Graphics/VEngine.hpp"
#include "Graphics/CommandBuffer.hpp"
#include "Graphics/Material.hpp"
#include "Type/ShaderTypes.hpp"

class Component;
class Transform;

class EngineObject {
private:
    vector<Component*> components;
    vector<EngineObject*> children;
    CommandBuffer* commandBuffer;
    VEngine* engine = nullptr;
public:
    EngineObject* parent = nullptr;
    Transform* transform;

    EngineObject();

    void SetEngine(VEngine* engine);
    inline VEngine* GetEngine() { return engine; }
    inline CommandBuffer* GetCommandBuffer() { return commandBuffer; }

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