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
#include "Deferred/DeferredHandler.hpp"

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

    auto defaultRender = RenderPassManager::GetRenderPass("default", this);
    defaultRender->Prepare(swapchain->size, swapchain->format);
    defaultRender->Create();

    renderPass = new VRenderPass(allocator, device);
    renderPass->Prepare(swapchain->size, swapchain->format);
    renderPass->Create();

    framebuffer = new VFramebuffer();
    framebuffer->Create(swapchain, renderPass);
}

void VEngine::Create(Window* _window) {
    window = _window;

    CreateBase();

    clearCommand = new CommandBuffer(this);
    clearCommand->renderPass = renderPass;
    clearCommand->Create();

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        clearCommand->Begin(i);
        clearCommand->LoadDefault(i);
        clearCommand->End();
    }

    MaterialManager::GetMaterial("default");
}

void VEngine::CreateDeferred(Window* _window) {
    window = _window;

    CreateBase();

    deferred = new DeferredHandler(this);
    deferred->Load();


    auto deferreClear = RenderPassManager::GetRenderPass("DeferredClear", this);
    deferreClear->Prepare(deferred->textures, false);
    deferreClear->Create();

    clearCommand = new CommandBuffer(this);
    clearCommand->renderPass = deferreClear;
    clearCommand->Create();
    clearCommand->vCommandBuffer->frame = &deferred->framebuffer;

    for (int i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        clearCommand->Begin(i);
        clearCommand->LoadDefault(i);
        clearCommand->End();
    }
}

void VEngine::PrepareDraw() {
    deferred->Update();
    sync->Wait(renderFrame);
    VK_CHECK(vkAcquireNextImageKHR(device->rawDevice, swapchain->rawSwapchain, 1000000000, sync->presents[renderFrame], nullptr, &imageIndex));
    if (clearCommand)
        drawQueue.push_back(clearCommand->GetCommandBuffer());

}

void VEngine::Draw() {
    if (deferred)
        drawQueue.push_back(deferred->commandBuffer->GetCommandBuffer());

    finalQueue.resize(drawQueue.size() + transQueue.size());
    memcpy(finalQueue.data(), drawQueue.data(), drawQueue.size() * sizeof(VkCommandBuffer));
    vector<VkCommandBuffer> finalTrans;

    for (auto d : transQueue)
        finalTrans.push_back(d.second);

    memcpy(&finalQueue[drawQueue.size()], finalTrans.data(), finalTrans.size() * sizeof(VkCommandBuffer));

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &sync->presents[renderFrame];

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &sync->renders[renderFrame];

    submit.commandBufferCount = finalQueue.size();
    submit.pCommandBuffers = finalQueue.data();

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
    finalQueue.resize(0);
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