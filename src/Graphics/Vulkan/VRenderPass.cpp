#include "VRenderPass.hpp"
#include "VDevice.hpp"
#include "VDepth.hpp"

VRenderPass::VRenderPass(VmaAllocator& _allocator, VDevice* _device) {
    device = _device;
    allocator = _allocator;
}

void VRenderPass::Prepare(Vector2 size, VkFormat format) {
    depth = new VDepth(allocator, device, size);
    depth->Create();

    descriptions.resize(2, {});

    descriptions[0].format = format;
    descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    descriptions[1].format = depth->format;
    descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments.resize(2, {});

    attachments[0].attachment = 0;
    attachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].attachment = 1;
    attachments[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachments[0];
    subpass.pDepthStencilAttachment = &attachments[1];

    dependencies.resize(2, {});

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = descriptions.size();
    renderPassInfo.pAttachments = descriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();
}

void VRenderPass::Create() {
    VK_CHECK(vkCreateRenderPass(device->rawDevice, &renderPassInfo, nullptr, &rawRenderPass));
}

void VRenderPass::Clean() const {
    vkDestroyRenderPass(device->rawDevice, rawRenderPass, nullptr);
}