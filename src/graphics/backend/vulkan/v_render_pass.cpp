#include <MVRE/graphics/backend/vulkan/v_render_pass.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_swapchain.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_framebuffer.hpp>

using namespace mvre_graphics;

void v_render_pass::begin() {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_render_pass;
    renderPassInfo.framebuffer = instance<v_backend_instance>()->framebuffer()->get_frame();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = instance<v_backend_instance>()->swapchain()->extent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(instance<v_backend_instance>()->raw_command_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void v_render_pass::end() {
    vkCmdEndRenderPass(instance<v_backend_instance>()->raw_command_buffer());
}

void v_render_pass::destroy() {
    vkDestroyRenderPass(instance<v_backend_instance>()->device()->raw_device(), m_render_pass, nullptr);
}

void v_render_pass::create() {
    auto v_instance = instance<v_backend_instance>();

    VkAttachmentDescription color_attachment{
        .format = v_instance->swapchain()->image_format(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference color_attachment_ref {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref
    };

    VkSubpassDependency dependency {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo render_pass_info {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    if (vkCreateRenderPass(v_instance->device()->raw_device(), &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Render Pass - Vulkan - Failed to create a render pass");
}