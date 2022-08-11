#include "QuickSubmit.hpp"
#include "VDevice.hpp"
#include "VCommandBuffer.hpp"
#include "VCommandPool.hpp"

QuickSubmit::QuickSubmit(VDevice* _device) {
    device = _device;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    vkCreateFence(device->rawDevice, &fenceCreateInfo, nullptr, &rawFence);

    commandPool = new VCommandPool(device);
    commandPool->Create();

    commandBuffer = new VCommandBuffer(commandPool);
    commandBuffer->Create(1);
}

void QuickSubmit::Begin() {
    commandBuffer->Begin(0);
}

void QuickSubmit::End() {
    commandBuffer->EndSingle();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer->rawCommandBuffers.data();
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    VK_CHECK(vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, rawFence));
    vkWaitForFences(device->rawDevice, 1, &rawFence, true, 9999999999);
    vkResetFences(device->rawDevice, 1, &rawFence);
    vkResetCommandPool(device->rawDevice, commandPool->rawCommandPool, 0);
}

void QuickSubmit::Clean() {
    vkDestroyFence(device->rawDevice, rawFence, nullptr);
}