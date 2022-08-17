#ifndef __COMPONENT__
#define __COMPONENT__

#include "Graphics/VEngine.hpp"
#include "Graphics/CommandBuffer.hpp"
#include "Components/Transform.hpp"

class EngineObject;

class Component {
public:
    bool isRenderer = false;
    EngineObject* object = nullptr;
    CommandBuffer* commandBuffer = nullptr;
    Material* material = nullptr;

    inline Transform* transform() { return object->transform; };
    inline VEngine* GetEngine() const { return object->GetEngine(); }

    inline CommandBuffer* GetCommandBuffer() {
        if (commandBuffer == nullptr) {
            commandBuffer = new CommandBuffer(GetEngine());
            commandBuffer->Create();
        }

        return commandBuffer;
    }

    Component();

    virtual void PreLoad();
    virtual void Load();
    virtual void PreRender();
    virtual void Update();
    virtual void Render();
    virtual void PostRender();
    virtual void Clean();
};

#endif