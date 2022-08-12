#ifndef __COMPONENT__
#define __COMPONENT__

#include "Graphics/VEngine.hpp"
#include "Components/Transform.hpp"

class EngineObject;

class Component {
public:
    EngineObject* object;

    inline Transform* transform() { return object->transform; };

    VEngine* GetEngine() const;

    virtual void PreLoad();
    virtual void Load();
    virtual void PreRender();
    virtual void Update();
    virtual void Render();
    virtual void PostRender();
    virtual void Clean();
};

#endif