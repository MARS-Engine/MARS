#include "VBuffer.hpp"
#include "VDevice.hpp"
#include "VInstance.hpp"
#include "VCommandBuffer.hpp"
#include "QuickSubmit.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VBuffer::VBuffer(VDevice* _device, VmaAllocator& _allocator) {
    allocator = _allocator;
    device = _device;
}

VmaAllocator VBuffer::GenerateAllocator(VDevice* device, VInstance* instance) {
    VmaAllocator _allocator;
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = device->physicalDevice;
    allocatorInfo.device = device->rawDevice;
    allocatorInfo.instance = instance->rawInstance;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
    return _allocator;
}

void VBuffer::Create(size_t size, int usage, VmaMemoryUsage memoryUsage) {
    bufferSize = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, nullptr));
}

void VBuffer::CreateImage(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.extent = extent;
    imageExtent = extent;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &imageAllocation, nullptr);
}

void VBuffer::CopyBuffer(VBuffer* src) const {
    auto* submit = new QuickSubmit(device);
    submit->Begin();
    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = bufferSize;
    vkCmdCopyBuffer(submit->commandBuffer->rawCommandBuffers[0], src->buffer, buffer, 1, &copy);
    submit->End();
    submit->Clean();
}

void VBuffer::CopyBufferImage() const {
    auto* submit = new QuickSubmit(device);
    submit->Begin();

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = imageExtent;

    vkCmdCopyBufferToImage(submit->commandBuffer->rawCommandBuffers[0], buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    submit->End();
    submit->Clean();
}

void VBuffer::Update(void* data) const {
    void* tempData;
    vmaMapMemory(allocator, allocation, &tempData);
    memcpy(tempData, data, bufferSize);
    vmaUnmapMemory(allocator, allocation);
}

void VBuffer::TransitionImageLayout(bool begin) {
    auto* submit = new QuickSubmit(device);
    submit->Begin();

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

    if (begin) {
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

    vkCmdPipelineBarrier(submit->commandBuffer->rawCommandBuffers[0], begin ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT, begin ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    submit->End();
    submit->Clean();
}


void VBuffer::Bind(VCommandBuffer* commandBuffer, VBufferBindType type) {
    VkDeviceSize offset = 0;
    switch (type) {
        case VBUFFER_VERTEX_BIND:
            return vkCmdBindVertexBuffers(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], 0, 1, &buffer, &offset);
        case VBUFFER_INDEX_BIND:
            return vkCmdBindIndexBuffer(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}