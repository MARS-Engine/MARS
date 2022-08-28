#ifndef __DEFERRED__HANDLER__
#define __DEFERRED__HANDLER__

#include "Manager/PipelineManager.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Manager/ShaderManager.hpp"
#include "../ShaderData.hpp"
#include "../Vulkan/VTypes.hpp"

#include "RendererBase.hpp"
#include "../Vulkan/VFramebuffer.hpp"

class DeferredRenderer : public  RendererBase {
public:
    ShaderData* data;
    vector<Texture*> texturesData;
    vector<DeferredTexture> textures;
    VFramebuffer* framebuffer;

    using RendererBase::RendererBase;

    void CreateTexture(const string& name, DEFERRED_TEXTURE_TYPE type) override;
    VRenderPass* GetRenderPass() override;
    VkFramebuffer GetFramebuffer(size_t index) override;
    void Clear() override;
    void Load() override;
    void Update() override;
    void Render() override;
};

#endif