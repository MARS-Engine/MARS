#include "VFramebuffer.hpp"
#include "VSwapchain.hpp"
#include "VRenderPass.hpp"
#include "VDevice.hpp"
#include "VDepth.hpp"
#include "VTexture.hpp"

void VFramebuffer::AddAttachment(VkImageView imageView) {
    attachments.push_back(imageView);
}

void VFramebuffer::SetAttachments(vector<Texture*> textures) {
    attachments.resize(textures.size());

    for (auto i = 0; i < textures.size(); i++)
        attachments[i] = textures[i]->vTexture->imageView;
}

void VFramebuffer::ClearAttachments() {
    attachments.clear();
}

void VFramebuffer::Create(VRenderPass* _renderPass, Vector2 size) {
    renderPass = _renderPass;

    VkFramebufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    bufferInfo.renderPass = renderPass->rawRenderPass;
    bufferInfo.pAttachments = attachments.data();
    bufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    bufferInfo.width = static_cast<uint32_t>(size.x);
    bufferInfo.height = static_cast<uint32_t>(size.y);
    bufferInfo.layers = 1;

    VkFramebuffer buffer;
    VK_CHECK(vkCreateFramebuffer(renderPass->device->rawDevice, &bufferInfo, nullptr, &buffer));
    rawFramebuffers.push_back(buffer);
}

void VFramebuffer::Clean() {
    for (auto& rawFramebuffer : rawFramebuffers)
        vkDestroyFramebuffer(renderPass->device->rawDevice, rawFramebuffer, nullptr);
}