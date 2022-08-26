#include "SimpleRenderer.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Graphics/Window.hpp"
#include "Graphics/Vulkan/VSwapchain.hpp"
#include "Graphics/Vulkan/VFramebuffer.hpp"

VkFramebuffer SimpleRenderer::GetFramebuffer(size_t index) {
    return engine->framebuffer->rawFramebuffers[index];
}

void SimpleRenderer::Load() {
    auto renderer = RenderPassManager::GetRenderPass("Renderer", engine);
    renderer->Prepare(engine->window->size, engine->swapchain->format);
    renderer->Create();
    
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