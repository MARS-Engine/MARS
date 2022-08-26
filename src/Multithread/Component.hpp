#ifndef __COMPONENT__
#define __COMPONENT__

#include "Graphics/VEngine.hpp"
#include "Graphics/CommandBuffer.hpp"
#include "Components/Transform.hpp"

class EngineObject;

class Component {
public:
    bool loaded = false;
    bool isRenderer = false;
    EngineObject* object = nullptr;

    inline Transform* transform() const { return object->transform; };
    inline VEngine* GetEngine() const { return object->GetEngine(); }
    inline Material* GetMaterial() const { return object->material; }
    inline void SetMaterial(Material* mat) const { object->material = mat; }

    inline CommandBuffer* GetCommandBuffer() {
        return object->GetCommandBuffer();
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