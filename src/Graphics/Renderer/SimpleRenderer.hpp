#ifndef __SIMPLE__RENDERER__
#define __SIMPLE__RENDERER__

#include "RendererBase.hpp"

class SimpleRenderer : public RendererBase{
public:
    using RendererBase::RendererBase;

    void Load() override;
    void Clear() override;
    void Render() override;
};

#endif