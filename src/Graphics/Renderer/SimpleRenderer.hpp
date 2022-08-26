#ifndef __SIMPLE__RENDERER__
#define __SIMPLE__RENDERER__

#include "RendererBase.hpp"

class SimpleRenderer : public RendererBase{
public:
    using RendererBase::RendererBase;

    VkFramebuffer GetFramebuffer(size_t index) override;
    void Load() override;
    void Clear() override;
};

#endif