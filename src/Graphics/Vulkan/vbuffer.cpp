#include "vbuffer.hpp"
#include "vdevice.hpp"
#include "vinstance.hpp"
#include "vcommand_buffer.hpp"
#include "quick_submit.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

vbuffer::vbuffer(vdevice* _device, VmaAllocator& _allocator) {
    allocator = _allocator;
    device = _device;
}

VmaAllocator vbuffer::GenerateAllocator(vdevice* _device, vinstance* _instance) {
    VmaAllocator _allocator;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _device->physical_device;
    allocatorInfo.device = _device->raw_device;
    allocatorInfo.instance = _instance->raw_instance;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
    return _allocator;
}

void vbuffer::Create(size_t _size, int _usage, VmaMemoryUsage _memory_usage) {
    buffer_size = _size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    
    bufferInfo.size = _size;
    bufferInfo.usage = _usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = _memory_usage;

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, nullptr));
}

void vbuffer::CreateImage(VkFormat _format, VkImageUsageFlags _usage, VkExtent3D _extent) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = _extent.width;
    imageInfo.extent.height = _extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = _format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = _usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.extent = _extent;
    image_extent = _extent;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &image_allocation, nullptr);
}

void vbuffer::CopyBuffer(vbuffer* _src) const {
    auto* submit = new quick_submit(device);
    submit->begin();
    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = buffer_size;
    vkCmdCopyBuffer(submit->command_buffer->raw_command_buffers[0], _src->buffer, buffer, 1, &copy);
    submit->end();
    submit->clean();
}

void vbuffer::CopyBufferImage() const {
    auto* submit = new quick_submit(device);
    submit->begin();

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = image_extent;

    vkCmdCopyBufferToImage(submit->command_buffer->raw_command_buffers[0], buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    submit->end();
    submit->clean();
}

void vbuffer::Update(void* _data) const {
    void* tempData;
    vmaMapMemory(allocator, allocation, &tempData);
    memcpy(tempData, _data, buffer_size);
    vmaUnmapMemory(allocator, allocation);
}

void vbuffer::TransitionImageLayout(bool _begin) {
    auto* submit = new quick_submit(device);
    submit->begin();

    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.subresourceRange = range;

    if (_begin) {
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else {
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }

    vkCmdPipelineBarrier(submit->command_buffer->raw_command_buffers[0], _begin ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT, _begin ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    submit->end();
    submit->clean();
}


void vbuffer::Bind(vcommand_buffer* _command_buffer, vbuffer_bind_type _type) {
    VkDeviceSize offset = 0;
    switch (_type) {
        case VBUFFER_VERTEX_BIND:
            return vkCmdBindVertexBuffers(_command_buffer->raw_command_buffers[_command_buffer->record_index], 0, 1, &buffer, &offset);
        case VBUFFER_INDEX_BIND:
            return vkCmdBindIndexBuffer(_command_buffer->raw_command_buffers[_command_buffer->record_index], buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}