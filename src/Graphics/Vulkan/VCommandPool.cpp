#include "VCommandPool.hpp"
#include "VDevice.hpp"

VCommandPool::VCommandPool(VDevice* _device) {
    device = _device;
}

void VCommandPool::Create() {
    VkCommandPoolCreateInfo createInfo = {  };
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.queueFamilyIndex = device->graphicsQueueFamily;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(device->rawDevice, &createInfo, nullptr, &rawCommandPool));
}

void VCommandPool::Clean() {
    vkDestroyCommandPool(device->rawDevice, rawCommandPool, nullptr);
}