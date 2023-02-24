#include <MARS/graphics/backend/vulkan/v_backend/v_command_pool.hpp>

using namespace mars_graphics;

void v_command_pool::create() {
    auto indices = graphics()->device()->family_indices();

    VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family.value(),
    };

    if (vkCreateCommandPool(graphics()->device()->raw_device(), &poolInfo, nullptr, &m_command_pool) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Failed to create command pool");
}