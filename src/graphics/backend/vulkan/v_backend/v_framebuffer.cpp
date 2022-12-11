#include <MVRE/graphics/backend/vulkan/v_backend/v_framebuffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mvre_graphics;

void v_framebuffer::create(const std::vector<VkImageView>& _views) {
    m_framebuffers.resize(_views.size());

    VkFramebufferCreateInfo framebufferInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_render_pass->raw_render_pass(),
            .attachmentCount = 1,
            .width = m_size.x(),
            .height = m_size.y(),
            .layers = 1
    };

    for (int i = 0; i < _views.size(); i++) {
        framebufferInfo.pAttachments = &_views[i];

        if (vkCreateFramebuffer(graphics_instance()->device()->raw_device(), &framebufferInfo, nullptr, &m_framebuffers[i++]) != VK_SUCCESS)
            mvre_debug::debug::error("MVRE - Vulkan - Failed to create framebuffer");
    }
}

void v_framebuffer::destroy() {
    for (auto& frame : m_framebuffers)
        vkDestroyFramebuffer(graphics_instance()->device()->raw_device(), frame, nullptr);
}