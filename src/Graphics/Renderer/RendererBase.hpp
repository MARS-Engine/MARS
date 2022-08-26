#ifndef __RENDERER__BASE__
#define __RENDERER__BASE__

#include "Graphics/CommandBuffer.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Vulkan/VRenderPass.hpp"

class VEngine;

enum DEFERRED_TEXTURE_TYPE {
    COLOR,
    POSITION
};

struct DeferredTexture {
public:
    string name;
    DEFERRED_TEXTURE_TYPE type;
    Texture* texture;
};

class RendererBase {
public:
    CommandBuffer* clearBuffer = nullptr;
    CommandBuffer* renderBuffer = nullptr;
    VEngine* engine = nullptr;

    explicit RendererBase(VEngine* engine);

    virtual void CreateTexture(const string& name, DEFERRED_TEXTURE_TYPE type);
    virtual VRenderPass* GetRenderPass();
    virtual VkFramebuffer GetFramebuffer(size_t index);
    virtual void Clear();
    virtual void Load();
    virtual void Update();
    virtual void Render();
};

#endif