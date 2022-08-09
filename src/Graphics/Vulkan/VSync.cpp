#include "VSync.hpp"
#include "VDevice.hpp"
#include "../VEngine.hpp"

VSync::VSync(VDevice *_device) {
    device = _device;
}

void VSync::Create() {
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    fences.resize(VEngine::FRAME_OVERLAP);
    presents.resize(VEngine::FRAME_OVERLAP);
    renders.resize(VEngine::FRAME_OVERLAP);

    for (size_t i = 0; i < VEngine::FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(device->rawDevice, &fenceCreateInfo, nullptr, &fences[i]));

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        VK_CHECK(vkCreateSemaphore(device->rawDevice, &semaphoreCreateInfo, nullptr, &presents[i]));
        VK_CHECK(vkCreateSemaphore(device->rawDevice, &semaphoreCreateInfo, nullptr, &renders[i]));
    }
}

void VSync::Wait(size_t frame) {
    VK_CHECK(vkWaitForFences(device->rawDevice, 1, &fences[frame], true, 1000000000));
    VK_CHECK(vkResetFences(device->rawDevice, 1, &fences[frame]));
}

