#ifndef __ENGINE__OBJECT__
#define __ENGINE__OBJECT__

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
    CommandBuffer* commandBuffer = nullptr;
    VEngine* engine = nullptr;
public:
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