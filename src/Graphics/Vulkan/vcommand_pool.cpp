#include "vcommand_pool.hpp"
#include "vdevice.hpp"

vcommand_pool::vcommand_pool(vdevice* _device) {
    device = _device;
}

void vcommand_pool::create() {
    VkCommandPoolCreateInfo createInfo = {  };
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.queueFamilyIndex = device->graphics_queue_family;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(device->raw_device, &createInfo, nullptr, &raw_command_pool));
}

void vcommand_pool::clean() {
    vkDestroyCommandPool(device->raw_device, raw_command_pool, nullptr);
}