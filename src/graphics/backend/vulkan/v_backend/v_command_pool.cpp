#include <MVRE/graphics/backend/vulkan/v_backend/v_command_pool.hpp>

using namespace mvre_graphics;

void v_command_pool::create() {
    auto indices = graphics_instance()->device()->family_indices();

    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family.value(),
    };

    if (vkCreateCommandPool(graphics_instance()->device()->raw_device(), &poolInfo, nullptr, &m_command_pool) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Failed to create command pool");
}