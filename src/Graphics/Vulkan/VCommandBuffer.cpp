#include "VCommandBuffer.hpp"
#include "VCommandPool.hpp"
#include "VDevice.hpp"
#include "VRenderPass.hpp"
#include "VSwapchain.hpp"
#include "VFramebuffer.hpp"

VCommandBuffer::VCommandBuffer(VCommandPool* _commandPool) {
    commandPool = _commandPool;
    clearColor = Vector4(0, 0, 0, 0);
}

void VCommandBuffer::Create(size_t size) {
    rawCommandBuffers.resize(size);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = commandPool->rawCommandPool;
    allocInfo.commandBufferCount = size;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    rawCommandBuffers.resize(allocInfo.commandBufferCount);
    VK_CHECK(vkAllocateCommandBuffers(commandPool->device->rawDevice, &allocInfo, rawCommandBuffers.data()));
}
void VCommandBuffer::Reset(size_t index) {
    VK_CHECK(vkResetCommandBuffer(rawCommandBuffers[index], 0));
}

void VCommandBuffer::Begin(size_t index) {
    recordIndex = index;

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;

    VK_CHECK(vkBeginCommandBuffer(rawCommandBuffers[index], &cmdBeginInfo));
}

void VCommandBuffer::LoadDefault(VRenderPass* renderPass, VSwapchain* swapchain, VkFramebuffer framebuffer, size_t imageIndex) {
    VkClearValue clearValue;
    clearValue.color = { { clearColor.x, clearColor.y, clearColor.z, clearColor.z } };

    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.pNext = nullptr;

    rpInfo.renderPass = renderPass->rawRenderPass;
    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = { (uint32_t)swapchain->size.x, (uint32_t)swapchain->size.y };
    rpInfo.framebuffer = framebuffer;

    rpInfo.clearValueCount = renderPass->attachments.size();
    vector<VkClearValue> clearValues;
    clearValues.resize(renderPass->attachments.size());

    for (auto i = 0; i < clearValues.size(); i++)
        if (renderPass->attachments[i].layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            clearValues[i] = clearValue;
        else
            clearValues[i] = depthClear;

    rpInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(rawCommandBuffers[recordIndex], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VCommandBuffer::Draw(size_t vCount, size_t iCount) {
    vkCmdDraw(rawCommandBuffers[recordIndex], vCount, iCount, 0, 0);
}

void VCommandBuffer::DrawIndexed(size_t size, size_t iCount) {
    vkCmdDrawIndexed(rawCommandBuffers[recordIndex], size, iCount, 0, 0, 0);
}

void VCommandBuffer::End() {
    vkCmdEndRenderPass(rawCommandBuffers[recordIndex]);
    VK_CHECK(vkEndCommandBuffer(rawCommandBuffers[recordIndex]));
}

void VCommandBuffer::EndSingle() {
    VK_CHECK(vkEndCommandBuffer(rawCommandBuffers[recordIndex]));
}