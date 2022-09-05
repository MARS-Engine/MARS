#include "vrender_pass.hpp"
#include "vdevice.hpp"
#include "vdepth.hpp"
#include "vtexture.hpp"
#include "vbuffer.hpp"

VkImageLayout render_pass_data::get_initial_image_layout() {
    return should_load ? (swapchain_ready ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) : VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageLayout render_pass_data::get_final_image_layout() {
    return swapchain_ready ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

vrender_pass::vrender_pass(VmaAllocator& _allocator, vdevice* _device, render_pass_data _type) {
    device = _device;
    allocator = _allocator;
    type = _type;
}

void vrender_pass::add_description(VkFormat _format) {
    VkAttachmentDescription description{};

    description.samples = VK_SAMPLE_COUNT_1_BIT;
    description.loadOp = type.should_load ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
    description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    description.initialLayout = type.get_initial_image_layout();
    description.finalLayout = type.get_final_image_layout();
    description.format = _format;

    descriptions.push_back(description);

    VkAttachmentReference attachment;
    attachment.attachment = attachments.size();
    attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments.push_back(attachment);
}

void vrender_pass::prepare(vector<texture*> _textures) {

    for (size_t i = 0; i < _textures.size(); i++)
        add_description(_textures[i]->base_texture->format);

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

void vrender_pass::add_depth(vector2 _size) {
    depth = new vdepth(allocator, device, _size);
    depth->create();

    VkAttachmentDescription depthDesc{};
    depthDesc.samples = VK_SAMPLE_COUNT_1_BIT;
    depthDesc.format = depth->format;
    depthDesc.loadOp = type.should_load ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthDesc.initialLayout = type.should_load ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    descriptions.push_back(depthDesc);

    attachments.push_back({ static_cast<uint32_t>(attachments.size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
    subpass.pDepthStencilAttachment = &attachments[attachments.size() - 1];
}

void vrender_pass::create() {

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = attachments.data();
    subpass.colorAttachmentCount = static_cast<uint32_t>(attachments.size() - 1);

    render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = descriptions.size();
    render_pass_info.pAttachments = descriptions.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = dependencies.size();
    render_pass_info.pDependencies = dependencies.data();

    VK_CHECK(vkCreateRenderPass(device->raw_device, &render_pass_info, nullptr, &raw_render_pass));
}

void vrender_pass::clean() const {
    vkDestroyRenderPass(device->raw_device, raw_render_pass, nullptr);
}