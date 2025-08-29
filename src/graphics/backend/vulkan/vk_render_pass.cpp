#include <mars/graphics/backend/vulkan/vk_render_pass.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_swapchain.hpp>
#include <mars/meta.hpp>

#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_render_pass, 16> render_passes;
        log_channel render_pass_channel("graphics/vulkan/render_pass");
    } // namespace detail

    render_pass vk_render_pass_impl::vk_render_pass_create(const device& _device, const swapchain& _swapchain) {
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_swapchain* swapchain_ptr = static_cast<vk_swapchain*>(_swapchain.data);
        vk_render_pass* render_pass_ptr = detail::render_passes.request_entry();

        render_pass result;
        result.data = render_pass_ptr;
        result.engine = _device.engine;

        VkAttachmentDescription color_attachment{
            .format = swapchain_ptr->swapchain_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference color_attachment_ref{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_ref,
        };

        VkRenderPassCreateInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &color_attachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
        };

        VkResult vk_result = vkCreateRenderPass(device_ptr->device, &render_pass_info, nullptr, &render_pass_ptr->vk_render_pass);

        logger::assert_(vk_result == VK_SUCCESS, detail::render_pass_channel, "failed to create render pass with error {}", meta::enum_to_string(vk_result));

        return result;
    }

    void vk_render_pass_impl::vk_render_pass_destroy(render_pass& _render_pass, const device& _device) {
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_render_pass* render_pass_ptr = static_cast<vk_render_pass*>(_render_pass.data);
        vkDestroyRenderPass(device_ptr->device, render_pass_ptr->vk_render_pass, nullptr);
        _render_pass = {};
    }
} // namespace mars::graphics::vulkan