#include <MVRE/graphics/backend/vulkan/v_command_buffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_command_pool.hpp>

using namespace mvre_graphics;

void v_command_buffer::create() {
    m_command_buffer.resize(instance()->max_frames());

    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = instance<v_backend_instance>()->command_pool()->raw_command_pool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32_t)m_command_buffer.size(),
    };

    if (vkAllocateCommandBuffers(instance<v_backend_instance>()->device()->raw_device(), &allocInfo, m_command_buffer.data()) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Failed to allocate command buffers");
}

void v_command_buffer::destroy() {
    vkFreeCommandBuffers(instance<v_backend_instance>()->device()->raw_device(), instance<v_backend_instance>()->command_pool()->raw_command_pool(), m_command_buffer.size(), m_command_buffer.data());
}