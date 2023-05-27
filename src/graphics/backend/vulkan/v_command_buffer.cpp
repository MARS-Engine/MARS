#include <MARS/graphics/backend/vulkan/v_command_buffer.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_command_pool.hpp>

using namespace mars_graphics;

void v_command_buffer::create() {
    m_command_buffer.resize(m_size);

    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cast_graphics<vulkan_backend>()->command_pool()->raw_command_pool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t)m_command_buffer.size(),
    };

    if (vkAllocateCommandBuffers(cast_graphics<vulkan_backend>()->get_device()->raw_device(), &allocInfo, m_command_buffer.data()) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Failed to allocate command buffers");
}

v_command_buffer::~v_command_buffer() {
    vkFreeCommandBuffers(cast_graphics<vulkan_backend>()->get_device()->raw_device(), cast_graphics<vulkan_backend>()->command_pool()->raw_command_pool(), m_command_buffer.size(), m_command_buffer.data());
}