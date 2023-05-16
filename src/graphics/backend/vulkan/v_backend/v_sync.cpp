#include <MARS/graphics/backend/vulkan/v_backend/v_sync.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mars_graphics;

void v_sync::create() {
    m_image_available.resize(graphics()->max_frames());
    m_render_finished.resize(graphics()->max_frames());
    m_inflight_fence.resize(graphics()->max_frames());

    VkSemaphoreCreateInfo semaphore_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fence_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (size_t i = 0; i < m_image_available.size(); i++) {
        if (vkCreateSemaphore(graphics()->get_device()->raw_device(), &semaphore_info, nullptr, &m_image_available[i]) != VK_SUCCESS ||
            vkCreateSemaphore(graphics()->get_device()->raw_device(), &semaphore_info, nullptr, &m_render_finished[i]) != VK_SUCCESS ||
            vkCreateFence(graphics()->get_device()->raw_device(), &fence_info, nullptr, &m_inflight_fence[i]) != VK_SUCCESS)
            mars_debug::debug::error("MARS - Vulkan - Failed to create semaphores");
    }
}

void v_sync::wait() {
    vkWaitForFences(graphics()->get_device()->raw_device(), 1, &m_inflight_fence[graphics()->current_frame()], VK_TRUE, UINT64_MAX);
}

void v_sync::destroy() {
    for (size_t i = 0; i < m_image_available.size(); i++) {
        vkDestroySemaphore(graphics()->get_device()->raw_device(), m_image_available[i], nullptr);
        vkDestroySemaphore(graphics()->get_device()->raw_device(), m_render_finished[i], nullptr);
        vkDestroyFence(graphics()->get_device()->raw_device(), m_inflight_fence[i], nullptr);
    }
}