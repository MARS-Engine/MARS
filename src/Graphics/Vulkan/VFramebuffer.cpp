#include "VFramebuffer.hpp"
#include "VSwapchain.hpp"
#include "VRenderPass.hpp"
#include "VDevice.hpp"
#include "VDepth.hpp"

VFramebuffer::VFramebuffer(VSwapchain* _swapchain, VRenderPass* _renderPass) {
    swapchain = _swapchain;
    renderPass = _renderPass;
}

void VFramebuffer::Create() {
    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.renderPass = renderPass->rawRenderPass;
    createInfo.attachmentCount = 1;
    createInfo.width = swapchain->size.x;
    createInfo.height = swapchain->size.y;
    createInfo.layers = 1;

    rawFramebuffers.resize(swapchain->images.size());

    for (size_t i = 0; i < rawFramebuffers.size(); i++) {
        VkImageView attachments[2];
        attachments[0] = swapchain->imageViews[i];
        attachments[1] = renderPass->depth->imageView;

        createInfo.pAttachments = attachments;
        createInfo.attachmentCount = 2;

        VK_CHECK(vkCreateFramebuffer(renderPass->device->rawDevice, &createInfo, nullptr, &rawFramebuffers[i]));
    }
}

void VFramebuffer::Clean() {
    for (auto& rawFramebuffer : rawFramebuffers)
        vkDestroyFramebuffer(renderPass->device->rawDevice, rawFramebuffer, nullptr);
}