#include "RendererBase.hpp"
#include "../VEngine.hpp"

RendererBase::RendererBase(VEngine* _engine) {
    engine = _engine;
}

VRenderPass *RendererBase::GetRenderPass() { }
void RendererBase::CreateTexture(const string& name, DEFERRED_TEXTURE_TYPE type) {}
VkFramebuffer RendererBase::GetFramebuffer(size_t index) { }
void RendererBase::Clear() { }
void RendererBase::Load() { }
void RendererBase::Update() { }
void RendererBase::Render() { }