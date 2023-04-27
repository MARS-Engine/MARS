#include "MARS/graphics/backend/vulkan/v_framebuffer.hpp"
#include "MARS/graphics/backend/vulkan/v_backend/v_device.hpp"
#include "MARS/graphics/backend/vulkan/v_backend/v_depth.hpp"
#include "MARS/graphics/backend/vulkan/v_swapchain.hpp"
#include "MARS/graphics/backend/vulkan/v_type_helper.hpp"

using namespace mars_graphics;

void v_framebuffer::create(swapchain* _swapchain) {
    is_swap = true;


    if (m_depth_enabled) {
        m_depth = new v_depth(cast_graphics<vulkan_backend>());
        m_depth->create();
    }

    m_size = _swapchain->size();

    auto _views = ((v_swapchain*)_swapchain)->image_views();

    m_framebuffers.resize(_views.size());

    m_render_pass = graphics()->create<render_pass>();
    m_render_pass->set_load_previous(m_load_previous);
    m_render_pass->set_framebuffer(this);
    m_render_pass->add_attachment({ .format = VK2MARS(cast_graphics<vulkan_backend>()->swapchain()->image_format()), .layout = MARS_TEXTURE_LAYOUT_PRESENT });
    if (m_depth_enabled)
        m_render_pass->add_attachment({ .format = m_depth->get_format(), .layout = MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL });
    m_render_pass->create();

    VkFramebufferCreateInfo framebufferInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_render_pass.cast_static<v_render_pass>()->raw_render_pass(),
            .width = static_cast<uint32_t>(m_size.x),
            .height = static_cast<uint32_t>(m_size.y),
            .layers = 1
    };

    for (int i = 0; i < _views.size(); i++) {
        std::vector<VkImageView> attachments = {
            _views[i],
        };

        if (m_depth_enabled)
            attachments.push_back(m_depth->get_image_view());

        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.attachmentCount = attachments.size();

        if (vkCreateFramebuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            mars_debug::debug::error("MARS - Vulkan - Failed to create framebuffer");
    }
}
void v_framebuffer::create(mars_math::vector2<size_t> _size, const std::vector<mars_ref<texture>>& _textures) {
    if (m_depth_enabled) {
        m_depth = new v_depth(cast_graphics<vulkan_backend>());
        m_depth->create();
    }

    m_frames = _textures;

    m_size = _size;

    m_render_pass = graphics()->create<render_pass>();
    m_render_pass->set_framebuffer(this);

    std::vector<VkImageView> attachments;

    for (auto& _texture : _textures) {
        m_render_pass->add_attachment({ .format = _texture->format(), .layout = MARS_TEXTURE_LAYOUT_READONLY });
        attachments.push_back(_texture.cast_static<v_texture>()->raw_image_view());
    }

    if (m_depth_enabled) {
        attachments.push_back(m_depth->get_image_view());
        m_render_pass->add_attachment({ .format = m_depth->get_format(), .layout = MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL });
    }
    m_render_pass->create();

    VkFramebufferCreateInfo framebufferInfo {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_render_pass.cast_static<v_render_pass>()->raw_render_pass(),
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = static_cast<uint32_t>(m_size.x),
            .height = static_cast<uint32_t>(m_size.y),
            .layers = 1
    };

    m_framebuffers.resize(m_framebuffers.size() + 1);

    if (vkCreateFramebuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), &framebufferInfo, nullptr, &m_framebuffers[m_framebuffers.size() - 1]) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Failed to create framebuffer");
}

void v_framebuffer::destroy() {
    if (m_render_pass.is_alive())
        m_render_pass->destroy();

    if (m_depth_enabled) {
        m_depth->destroy();
        delete m_depth;
    }

    for (auto& frame : m_framebuffers)
        vkDestroyFramebuffer(cast_graphics<vulkan_backend>()->device()->raw_device(), frame, nullptr);
}