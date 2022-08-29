#include "vsync.hpp"
#include "vdevice.hpp"
#include "../vengine.hpp"

vsync::vsync(vdevice *_device) {
    device = _device;
}

void vsync::create() {
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    fences.resize(vengine::frame_overlap);
    presents.resize(vengine::frame_overlap);
    renders.resize(vengine::frame_overlap);

    for (size_t i = 0; i < vengine::frame_overlap; i++) {
        VK_CHECK(vkCreateFence(device->raw_device, &fenceCreateInfo, nullptr, &fences[i]));

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        VK_CHECK(vkCreateSemaphore(device->raw_device, &semaphoreCreateInfo, nullptr, &presents[i]));
        VK_CHECK(vkCreateSemaphore(device->raw_device, &semaphoreCreateInfo, nullptr, &renders[i]));
    }
}

void vsync::wait(size_t _frame) {
    VK_CHECK(vkWaitForFences(device->raw_device, 1, &fences[_frame], true, 1000000000));
    VK_CHECK(vkResetFences(device->raw_device, 1, &fences[_frame]));
}

