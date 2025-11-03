#include "mars/graphics/backend/texture.hpp"
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_framebuffer.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_render_pass.hpp>
#include <mars/graphics/backend/vulkan/vk_swapchain.hpp>
#include <mars/meta.hpp>

#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_framebuffer, 6> framebuffers;
        log_channel framebuffer_channel("graphics/vulkan/framebuffer");
    } // namespace detail

    framebuffer vk_framebuffer_impl::vk_framebuffer_create(const device& _device, const framebuffer_create_params& _params) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_render_pass* render_pass_ptr = _params.render_pass.data;
        vk_framebuffer* framebuffer_ptr = detail::framebuffers.request_entry();

        framebuffer result;
        result.data = framebuffer_ptr;
        result.engine = _device.engine;
        result.extent = { _params.size.x, _params.size.y };

        std::vector<VkImageView> views;

        _params.view.view_extractor.operator()<void, const texture_view&, std::vector<VkImageView>&>(_params.view, views);

        framebuffer_ptr->framebuffers.resize(views.size());

        for (size_t i = 0; i < framebuffer_ptr->framebuffers.size(); i++) {
            VkImageView attachments[] = { views[i] };

            VkFramebufferCreateInfo framebufferInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = render_pass_ptr->vk_render_pass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = static_cast<uint32_t>(_params.size.x),
                .height = static_cast<uint32_t>(_params.size.y),
                .layers = 1,
            };

            VkResult vk_result = vkCreateFramebuffer(device_ptr->device, &framebufferInfo, nullptr, &framebuffer_ptr->framebuffers[i]);

            logger::assert_(vk_result == VK_SUCCESS, detail::framebuffer_channel, "failed to create frame buffer with error {} for image index {}", meta::enum_to_string(vk_result), i);
        }

        return result;
    }

    void vk_framebuffer_impl::vk_framebuffer_destroy(framebuffer& _framebuffer, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_framebuffer* framebuffer_ptr = _framebuffer.data.get<vk_framebuffer>();

        for (VkFramebuffer vk_framebuffer : framebuffer_ptr->framebuffers)
            vkDestroyFramebuffer(device_ptr->device, vk_framebuffer, nullptr);

        detail::framebuffers.remove(framebuffer_ptr);
        _framebuffer = {};
    }
} // namespace mars::graphics::vulkan