#include "SimpleRenderer.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Vulkan/VSwapchain.hpp"

void SimpleRenderer::Load() {
    auto renderer = RenderPassManager::GetRenderPass("Renderer", engine);
    renderer->Prepare(engine->window->size, engine->swapchain->format);
    renderer->Create();

    renderBuffer = new CommandBuffer(engine);
    renderBuffer->renderPass = engine->renderPass;
    renderBuffer->Create();

    auto winSize = engine->window->size;

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        renderBuffer->Begin(i);
        renderBuffer->LoadDefault(i);

        //VkViewport viewport = {};
        //viewport.x = 0.0f;
        //viewport.y = winSize.y;
        //viewport.width = winSize.x;
        //viewport.height = -winSize.y;
        //viewport.minDepth = 0.0f;
        //viewport.maxDepth = 1.0f;
        //vkCmdSetViewport(renderBuffer->vCommandBuffer->rawCommandBuffers[i], 0, 1, &viewport);

        renderBuffer->End();
    }

    clearBuffer = new CommandBuffer(engine);
    clearBuffer->renderPass = engine->renderPass;
    clearBuffer->Create();

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        clearBuffer->Begin(i);
        clearBuffer->LoadDefault(i);
        clearBuffer->End();
    }
}
void SimpleRenderer::Clear() {
    engine->drawQueue.push_back(clearBuffer->GetCommandBuffer());
}

void SimpleRenderer::Render() {
    //engine->drawQueue.push_back(renderBuffer->GetCommandBuffer());
}