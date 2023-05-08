#include <MARS/graphics/backend/vulkan/v_render_pass.hpp>
#include "MARS/graphics/backend/vulkan/v_swapchain.hpp"
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include "MARS/graphics/backend/vulkan/v_framebuffer.hpp"
#include <MARS/graphics/backend/vulkan/v_backend/v_depth.hpp>
#include <MARS/graphics/backend/vulkan/v_type_helper.hpp>

using namespace mars_graphics;

v_render_pass::~v_render_pass() {
    vkDestroyRenderPass(cast_graphics<vulkan_backend>()->device()->raw_device(), m_render_pass, nullptr);
}

void v_render_pass::begin() {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_render_pass;
    renderPassInfo.framebuffer = m_data.framebuffer_ptr->cast<v_framebuffer>()->get_frame();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = cast_graphics<vulkan_backend>()->swapchain()->extent();

    std::vector<VkClearValue> clearValues;

    for (auto& attachment : m_data.attachments) {
        VkClearValue value;
        switch (attachment.layout) {
            default:
                value.color = { 0.0f, 0.0f, 0.0f , 1.0f };
                break;
            case MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL:
                value.depthStencil = { 1.0f, 0 };
                break;
        }

        clearValues.push_back(value);
    }

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cast_graphics<vulkan_backend>()->raw_command_buffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void v_render_pass::end() {
    vkCmdEndRenderPass(cast_graphics<vulkan_backend>()->raw_command_buffer());
}

void v_render_pass::create() {
    auto v_instance = cast_graphics<vulkan_backend>();

    std::vector<VkAttachmentDescription> vk_attachments;
    std::vector<VkAttachmentReference> vk_attachment_refs;
    VkAttachmentReference depth_ref {
        .layout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    for (uint32_t i = 0; auto attachment : m_data.attachments) {
        VkAttachmentDescription vk_attachment;
        switch (attachment.layout) {
            case MARS_TEXTURE_LAYOUT_PRESENT:
            default:
                vk_attachment = {
                        .format = MARS2VK(attachment.format),
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = MARS2VK(attachment.layout),
                };
                break;
            case MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL:
                vk_attachment = {
                        .format = MARS2VK(attachment.format),
                        .samples = VK_SAMPLE_COUNT_1_BIT,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout = MARS2VK(attachment.layout)
                };
                break;
        }

        vk_attachments.push_back(vk_attachment);

        VkAttachmentReference ref = {
            .attachment = i++,
            .layout = vk_attachment.finalLayout
        };

        if (attachment.layout == MARS_TEXTURE_LAYOUT_READONLY || attachment.layout == MARS_TEXTURE_LAYOUT_PRESENT)
            ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (attachment.layout != MARS_TEXTURE_LAYOUT_DEPTH_OPTIMAL)
            vk_attachment_refs.push_back(ref);
        else
            depth_ref = ref;
    }

    VkSubpassDescription subpass {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<uint32_t>(vk_attachment_refs.size()),
        .pColorAttachments = vk_attachment_refs.data(),
        .pDepthStencilAttachment = depth_ref.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr : &depth_ref
    };

    std::vector<VkSubpassDependency> dependecies;

    if (m_data.load_previous) {
        dependecies = {
                {
                        .srcSubpass = VK_SUBPASS_EXTERNAL,
                        .dstSubpass = 0,
                        .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                        .dependencyFlags = 0
                },
                {
                        .srcSubpass = VK_SUBPASS_EXTERNAL,
                        .dstSubpass = 0,
                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                        .dependencyFlags = 0
                }
        };
    }
    else {
        dependecies = {
                {
                        .srcSubpass = VK_SUBPASS_EXTERNAL,
                        .dstSubpass = 0,
                        .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
                },
                {
                        .srcSubpass = 0,
                        .dstSubpass = VK_SUBPASS_EXTERNAL,
                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
                }
        };
    }

    VkRenderPassCreateInfo render_pass_info {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(vk_attachments.size()),
        .pAttachments = vk_attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = static_cast<uint32_t>(dependecies.size()),
        .pDependencies = dependecies.data(),
    };

    if (vkCreateRenderPass(v_instance->device()->raw_device(), &render_pass_info, nullptr, &m_render_pass) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Render Pass - Vulkan - Failed to create a render pass");
}