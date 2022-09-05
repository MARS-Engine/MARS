#include "quick_submit.hpp"
#include "vdevice.hpp"
#include "vcommand_buffer.hpp"
#include "vcommand_pool.hpp"

quick_submit::quick_submit(vdevice* _device) {
    device = _device;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    vkCreateFence(device->raw_device, &fenceCreateInfo, nullptr, &raw_fence);

    command_pool = new vcommand_pool(device);
    command_pool->create();

    command_buffer = new vcommand_buffer(command_pool);
    command_buffer->create(1);
}

void quick_submit::begin() {
    command_buffer->begin(0);
}

void quick_submit::end() {
    command_buffer->end_single();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;

    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = command_buffer->raw_command_buffers.data();
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    VK_CHECK(vkQueueSubmit(device->graphics_queue, 1, &submitInfo, raw_fence));
    vkWaitForFences(device->raw_device, 1, &raw_fence, true, 9999999999);
    vkResetFences(device->raw_device, 1, &raw_fence);
    vkResetCommandPool(device->raw_device, command_pool->raw_command_pool, 0);
}

void quick_submit::clean() {
    vkDestroyFence(device->raw_device, raw_fence, nullptr);
}