#ifndef __COMPONENT__
#define __COMPONENT__

#include "Graphics/VEngine.hpp"

class EngineObject;

class Component {
public:
    EngineObject* object;

    VEngine* GetEngine() const;

    virtual void Load();
    virtual void PreRender();
    virtual void Update();
    virtual void Render();
    virtual void PostRender();
    virtual void Clean();
};

#endif