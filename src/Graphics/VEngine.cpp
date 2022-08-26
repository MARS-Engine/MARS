#include "VEngine.hpp"
#include "Vulkan/VDevice.hpp"
#include "Vulkan/VInstance.hpp"
#include "Vulkan/VSurface.hpp"
#include "Vulkan/VSwapchain.hpp"
#include "Vulkan/VCommandPool.hpp"
#include "Vulkan/VRenderPass.hpp"
#include "Vulkan/VFramebuffer.hpp"
#include "Vulkan/VSync.hpp"
#include "Vulkan/VBuffer.hpp"
#include "Manager/RenderPassManager.hpp"
#include "Manager/MaterialManager.hpp"
#include "Components/Graphics/Camera.hpp"
#include "Renderer/DeferredRenderer.hpp"
#include "Renderer/SimpleRenderer.hpp"

unsigned int VEngine::FRAME_OVERLAP = 2;

Camera* VEngine::GetCamera() {
    if (cameras.empty())
        return nullptr;
    return cameras[0];
}

void VEngine::CreateBase() {
    instance = new VInstance();
    instance->Create();

    surface = new VSurface(window, instance);
    surface->Create();

    device = new VDevice(instance, surface);
    device->Create();

    allocator = VBuffer::GenerateAllocator(device, instance);

    swapchain = new VSwapchain(device);
    swapchain->Create();

    FRAME_OVERLAP = swapchain->imageViews.size();

    commandPool = new VCommandPool(device);
    commandPool->Create();

    sync = new VSync(device);
    sync->Create();

    renderPass = RenderPassManager::GetRenderPass("default", this);
    renderPass->Prepare(swapchain->size, swapchain->format, false);
    renderPass->Create();

    framebuffer = new VFramebuffer();
    framebuffer->Create(swapchain, renderPass);
}

void VEngine::Create(RENDER_TYPE _type, Window* _window) {
    window = _window;
    type = _type;

    CreateBase();

    switch (type) {
        case SIMPLE:
            renderer = new SimpleRenderer(this);
            renderer->Load();
            break;
        case DEFERRED:
            renderer = new DeferredRenderer(this);
            renderer->CreateTexture("deferredPosition", POSITION);
            renderer->CreateTexture("deferredNormal", POSITION);
            renderer->CreateTexture("deferredAlbedo", COLOR);
            renderer->Load();
            break;
    }
}

VkFramebuffer VEngine::GetFramebuffer(int i) {
    return renderer->GetFramebuffer(i);
}

void VEngine::PrepareDraw() {
    renderer->Update();
    sync->Wait(renderFrame);
    VK_CHECK(vkAcquireNextImageKHR(device->rawDevice, swapchain->rawSwapchain, 1000000000, sync->presents[renderFrame], nullptr, &imageIndex));
    renderer->Clear();

}

void VEngine::Draw() {
    auto nonTransSize = drawQueue.size();
    drawQueue.resize(nonTransSize + transQueue.size());

    auto i = 0;
    for (auto t : transQueue)
        drawQueue[nonTransSize + i++] = t.second;

    if (renderer)
        renderer->Render();

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &sync->presents[renderFrame];

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &sync->renders[renderFrame];

    submit.commandBufferCount = drawQueue.size();
    submit.pCommandBuffers = drawQueue.data();

    VK_CHECK(vkQueueSubmit(device->graphicsQueue, 1, &submit, sync->fences[renderFrame]));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &swapchain->rawSwapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &sync->renders[renderFrame];
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &imageIndex;

    VK_CHECK(vkQueuePresentKHR(device->graphicsQueue, &presentInfo));

    renderFrame = (renderFrame + 1) % FRAME_OVERLAP;
    transQueue.clear();
    drawQueue.resize(0);
}

void VEngine::Clean() const {
    commandPool->Clean();
    renderPass->Clean();
    framebuffer->Clean();
    swapchain->Clean();
    device->Clean();
    surface->Clean();
    instance->Clean();
}