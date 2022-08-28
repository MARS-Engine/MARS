#ifndef __COMPONENT__
#define __COMPONENT__

#include "Graphics/VEngine.hpp"
#include "Graphics/CommandBuffer.hpp"
#include "Components/Transform.hpp"

class ComponentInterface {
public:
    bool isRenderer = false;
    bool loaded = false;
    EngineObject* object = nullptr;

    virtual size_t Size() const;
    virtual void PreLoad();
    virtual void Load();
    virtual void PreRender();
    virtual void Update();
    virtual void Render();
    virtual void PostRender();
    virtual void Clean();
};

template<typename Derived> class Component : public ComponentInterface {
public:
    inline Transform* GetTransform() const { return object->transform; };
    inline VEngine* GetEngine() const { return object->GetEngine(); }
    inline Material* GetMaterial() const { return object->material; }
    inline void SetMaterial(Material* mat) const { object->material = mat; }

    size_t Size() const override { return sizeof(Derived); }

    template<class T> inline T* GetComponent() { return object->GetComponent<T>(); }
    template<class T> inline T* AddComponent(T* comp) { return object->AddComponent(comp); }
    template<class T> inline T* AddComponent() { return object->AddComponent<T>(); }


    inline CommandBuffer* GetCommandBuffer() {
        return object->GetCommandBuffer();
    }
};

#endif