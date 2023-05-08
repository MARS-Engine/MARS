#include "MARS/graphics/backend/vulkan/v_framebuffer.hpp"
#include "MARS/graphics/backend/vulkan/v_backend/v_device.hpp"
#include "MARS/graphics/backend/vulkan/v_backend/v_depth.hpp"
#include "MARS/graphics/backend/vulkan/v_swapchain.hpp"
#include "MARS/graphics/backend/vulkan/v_type_helper.hpp"

using namespace mars_graphics;

void v_framebuffer::create(swapchain* _swapchain) {
    is_swap = true;


    if (m_data.depth_enabled) {
        m_depth = new v_depth(cast_graphics<vulkan_backend>());
        m_depth->create();
    }

    m_data.size = _swapchain->size();

    auto _views = ((v_swapchain*)_swapchain)->image_views();

    m_framebuffers.resize(_views.size());

    auto builder = graphics()->builder<render_pass_builder>();
    builder
        .set_load_previous(m_data.load_previous)
        .set_framebuffer(mars_ref<framebuffer>(weak_from_this()))
        .add_attachment({ .format = VK2MARS(cast_graphics<vulkan_backend>()->swapchain()->image_format()), .layout = MARS_TEXTURE_LAYOUT_PRESENT });

    if (m_data.depth_enabled)
        builder.add_attachment({ .format = m_depth->get_format(), .layout = MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL });

    m_render_pass = builder.build();

    VkFramebufferCreateInfo framebufferInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_render_pass->cast<v_render_pass>()->raw_render_pass(),
            .width = static_cast<uint32_t>(m_data.size.x),
            .height = static_cast<uint32_t>(m_data.size.y),
            .layers = 1
    };

    for (int i = 0; i < _views.size(); i++) {
        std::vector<VkImageView> attachments = {
            _views[i],
        };

        if (m_data.depth_enabled)
            attachments.push_back(m_depth->get_image_view());

        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.attachmentCount = attachments.size();

        if (vkCreateFramebuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            mars_debug::debug::error("MARS - Vulkan - Failed to create framebuffer");
    }
}
void v_framebuffer::create(const std::vector<std::shared_ptr<texture>>& _textures) {
    if (m_data.depth_enabled) {
        m_depth = new v_depth(cast_graphics<vulkan_backend>());
        m_depth->create();
    }

    m_frames = _textures;

    auto builder = graphics()->builder<render_pass_builder>();
    builder.set_framebuffer(mars_ref<framebuffer>(weak_from_this()));

    std::vector<VkImageView> attachments;

    for (auto& _texture : _textures) {
        builder.add_attachment({ .format = _texture->format(), .layout = MARS_TEXTURE_LAYOUT_READONLY });
        attachments.push_back(_texture->cast<v_texture>()->raw_image_view());
    }

    if (m_data.depth_enabled) {
        attachments.push_back(m_depth->get_image_view());
        builder.add_attachment({ .format = m_depth->get_format(), .layout = MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL });
    }
    m_render_pass = builder.build();

    VkFramebufferCreateInfo framebufferInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_render_pass->cast<v_render_pass>()->raw_render_pass(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = static_cast<uint32_t>(m_data.size.x),
            .height = static_cast<uint32_t>(m_data.size.y),
            .layers = 1
    };

    m_framebuffers.resize(m_framebuffers.size() + 1);

    if (vkCreateFramebuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), &framebufferInfo, nullptr, &m_framebuffers[m_framebuffers.size() - 1]) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Failed to create framebuffer");
}

v_framebuffer::~v_framebuffer() {
    m_render_pass = nullptr;

    if (m_data.depth_enabled) {
        m_depth->destroy();
        delete m_depth;
    }

    for (auto& frame : m_framebuffers)
        vkDestroyFramebuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), frame, nullptr);
}