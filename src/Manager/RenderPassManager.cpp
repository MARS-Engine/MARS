#include "RenderPassManager.hpp"

vector<VRenderPass*> RenderPassManager::renderPasses;

VRenderPass* RenderPassManager::GetRenderPass(const string& name, VEngine* engine) {
    for (auto renderPass : renderPasses)
        if (renderPass->name == name)
            return renderPass;
    auto renderPass = new VRenderPass(engine->allocator, engine->device);
    renderPasses.push_back(renderPass);
    renderPass->name = name;
    return renderPass;
}