#include "VCommandBuffer.hpp"
#include "VCommandPool.hpp"
#include "VDevice.hpp"
#include "VRenderPass.hpp"
#include "VSwapchain.hpp"
#include "VFramebuffer.hpp"

VCommandBuffer::VCommandBuffer(VCommandPool* _commandPool) {
    commandPool = _commandPool;
    clearColor = Vector4(0, 0, 0, 1);
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
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(rawCommandBuffers[index], &cmdBeginInfo));
}

void VCommandBuffer::LoadDefault(VRenderPass* renderPass, VSwapchain* swapchain, VFramebuffer* framebuffer, size_t imageIndex) {
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
    rpInfo.framebuffer = framebuffer->rawFramebuffers[imageIndex];

    rpInfo.clearValueCount = 2;
    VkClearValue clearValues[] = { clearValue, depthClear };
    rpInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(rawCommandBuffers[recordIndex], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VCommandBuffer::Draw(size_t vCount, size_t iCount) {
    vkCmdDraw(rawCommandBuffers[recordIndex], vCount, iCount, 0, 0);
}

void VCommandBuffer::End() {
    vkCmdEndRenderPass(rawCommandBuffers[recordIndex]);
    VK_CHECK(vkEndCommandBuffer(rawCommandBuffers[recordIndex]));
}

void VCommandBuffer::EndSingle() {
    VK_CHECK(vkEndCommandBuffer(rawCommandBuffers[recordIndex]));
}