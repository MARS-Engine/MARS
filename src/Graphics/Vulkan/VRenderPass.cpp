#include "VRenderPass.hpp"
#include "VDevice.hpp"
#include "VDepth.hpp"
#include "VTexture.hpp"
#include "VBuffer.hpp"

VkImageLayout RenderPassData::GetInitialImageLayout() {
    return shouldLoad ? (swapchainReady ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) : VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageLayout RenderPassData::GetFinalImageLayout() {
    return swapchainReady ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

VRenderPass::VRenderPass(VmaAllocator& _allocator, VDevice* _device, RenderPassData _type) {
    device = _device;
    allocator = _allocator;
    type = _type;
}

void VRenderPass::AddDescription(VkFormat format) {
    VkAttachmentDescription description{};

    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = type.shouldLoad ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = type.GetInitialImageLayout();
    description.finalLayout = type.GetFinalImageLayout();
    description.format = format;

    descriptions.push_back(description);

    VkAttachmentReference attachment;
    attachment.attachment = attachments.size();
    attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments.push_back(attachment);
}

void VRenderPass::Prepare(vector<Texture*> textures) {

    for (size_t i = 0; i < textures.size(); i++)
        AddDescription(textures[i]->vTexture->format);

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
}

void VRenderPass::AddDepth(Vector2 size) {
    depth = new VDepth(allocator, device, size);
    depth->Create();

    VkAttachmentDescription depthDesc{};
    depthDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    depthDesc.format = depth->format;
    depthDesc.loadOp = type.shouldLoad ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDesc.initialLayout = type.shouldLoad ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    descriptions.push_back(depthDesc);

    attachments.push_back({ static_cast<uint32_t>(attachments.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
    subpass.pDepthStencilAttachment = &attachments[attachments.size() - 1];
}

void VRenderPass::Create() {

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = attachments.data();
    subpass.colorAttachmentCount = static_cast<uint32_t>(attachments.size() - 1);

    renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = descriptions.size();
    renderPassInfo.pAttachments = descriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    VK_CHECK(vkCreateRenderPass(device->rawDevice, &renderPassInfo, nullptr, &rawRenderPass));
}

void VRenderPass::Clean() const {
    vkDestroyRenderPass(device->rawDevice, rawRenderPass, nullptr);
}