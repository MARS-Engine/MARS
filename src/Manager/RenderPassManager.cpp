#include "RenderPassManager.hpp"

vector<VRenderPass*> RenderPassManager::renderPasses;

VRenderPass* RenderPassManager::GetRenderPass(const string& name, VEngine* engine, RenderPassData type) {
    for (auto renderPass : renderPasses)
        if (renderPass->name == name)
            return renderPass;
    auto renderPass = new VRenderPass(engine->allocator, engine->device, type);
    renderPasses.push_back(renderPass);
    renderPass->name = name;
    return renderPass;
}