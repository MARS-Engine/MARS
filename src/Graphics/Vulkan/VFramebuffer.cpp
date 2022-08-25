#include "VFramebuffer.hpp"
#include "VSwapchain.hpp"
#include "VRenderPass.hpp"
#include "VDevice.hpp"
#include "VDepth.hpp"
#include "VTexture.hpp"

void VFramebuffer::Create(VSwapchain* _swapchain, VRenderPass* _renderPass) {
    swapchain = _swapchain;
    renderPass = _renderPass;

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

void VFramebuffer::Create(Vector2 size, vector<Texture*> textures) {
    vector<VkImageView> attachments;
    attachments.resize(textures.size());
    rawFramebuffers.resize(1);

    for (auto i = 0; i < textures.size(); i++)
        attachments[i] = textures[i]->vTexture->imageView;

    VkFramebufferCreateInfo fbufCreateInfo = {};
    fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbufCreateInfo.pNext = NULL;
    fbufCreateInfo.renderPass = renderPass->rawRenderPass;
    fbufCreateInfo.pAttachments = attachments.data();
    fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    fbufCreateInfo.width = size.x;
    fbufCreateInfo.height = size.y;
    fbufCreateInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(renderPass->device->rawDevice, &fbufCreateInfo, nullptr, rawFramebuffers.data()));
}

void VFramebuffer::Clean() {
    for (auto& rawFramebuffer : rawFramebuffers)
        vkDestroyFramebuffer(renderPass->device->rawDevice, rawFramebuffer, nullptr);
}