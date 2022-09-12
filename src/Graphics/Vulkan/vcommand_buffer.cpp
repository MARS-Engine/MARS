#include "vcommand_buffer.hpp"
#include "vcommand_pool.hpp"
#include "vdevice.hpp"
#include "vrender_pass.hpp"
#include "vswapchain.hpp"
#include "vframebuffer.hpp"

vcommand_buffer::vcommand_buffer(vcommand_pool* _commandPool) {
    command_pool = _commandPool;
    clear_color = vector4(0, 0, 0, 0);
}

void vcommand_buffer::create(size_t _size) {
    raw_command_buffers.resize(_size);
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = command_pool->raw_command_pool;
    allocInfo.commandBufferCount = _size;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    raw_command_buffers.resize(allocInfo.commandBufferCount);
    VK_CHECK(vkAllocateCommandBuffers(command_pool->device->raw_device, &allocInfo, raw_command_buffers.data()));
}
void vcommand_buffer::reset(size_t _index) {
    VK_CHECK(vkResetCommandBuffer(raw_command_buffers[_index], 0));
}

void vcommand_buffer::begin(size_t _index) {
    record_index = _index;

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;

    VK_CHECK(vkBeginCommandBuffer(raw_command_buffers[_index], &cmdBeginInfo));
}

void vcommand_buffer::load_default(vrender_pass* _render_pass, vswapchain* _swapchain, VkFramebuffer _framebuffer, size_t _image_index) {
    VkClearValue clearValue;
    clearValue.color = { {clear_color.x, clear_color.y, clear_color.z, clear_color.z } };

    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.pNext = nullptr;

    rpInfo.renderPass = _render_pass->raw_render_pass;
    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = {(uint32_t)_swapchain->size.x, (uint32_t)_swapchain->size.y };
    rpInfo.framebuffer = _framebuffer;

    rpInfo.clearValueCount = _render_pass->attachments.size();
    std::vector<VkClearValue> clearValues;
    clearValues.resize(_render_pass->attachments.size());

    for (auto i = 0; i < clearValues.size(); i++)
        if (_render_pass->attachments[i].layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            clearValues[i] = clearValue;
        else
            clearValues[i] = depthClear;

    rpInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(raw_command_buffers[record_index], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void vcommand_buffer::draw(size_t _v_count, size_t _i_count) {
    vkCmdDraw(raw_command_buffers[record_index], _v_count, _i_count, 0, 0);
}

void vcommand_buffer::draw_indexed(size_t _size, size_t _i_count) {
    vkCmdDrawIndexed(raw_command_buffers[record_index], _size, _i_count, 0, 0, 0);
}

void vcommand_buffer::end() {
    vkCmdEndRenderPass(raw_command_buffers[record_index]);
    VK_CHECK(vkEndCommandBuffer(raw_command_buffers[record_index]));
}

void vcommand_buffer::end_single() {
    VK_CHECK(vkEndCommandBuffer(raw_command_buffers[record_index]));
}