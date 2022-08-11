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
#include "Vulkan/VDescriptorPool.hpp"
void VEngine::Create(Window* _window) {
    window = _window;

    instance = new VInstance();
    instance->Create();

    surface = new VSurface(window, instance);
    surface->Create();

    device = new VDevice(instance, surface);
    device->Create();

    allocator = VBuffer::GenerateAllocator(device, instance);

    swapchain = new VSwapchain(device);
    swapchain->Create();

    commandPool = new VCommandPool(device);
    commandPool->Create();

    commandBuffer = new CommandBuffer(this);
    commandBuffer->Create();

    renderPass = new VRenderPass(allocator, device);
    renderPass->Create(swapchain->size, swapchain->format);

    framebuffer = new VFramebuffer(swapchain, renderPass);
    framebuffer->Create();

    sync = new VSync(device);
    sync->Create();
}

void VEngine::PrepareDraw() {
    sync->Wait(renderFrame);
    VK_CHECK(vkAcquireNextImageKHR(device->rawDevice, swapchain->rawSwapchain, 1000000000, sync->presents[renderFrame], nullptr, &imageIndex));
    commandBuffer->Reset();
    commandBuffer->Begin();
    commandBuffer->LoadDefault();
}
void VEngine::Draw() {
    commandBuffer->End();

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &sync->presents[renderFrame];

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &sync->renders[renderFrame];

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &commandBuffer->vCommandBuffer->rawCommandBuffers[renderFrame];

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