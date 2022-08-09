#include "VDepth.hpp"
#include "VSwapchain.hpp"
#include "VDevice.hpp"

VDepth::VDepth(VmaAllocator& _allocator, VDevice* _device, Vector2 _size) {
    device = _device;
    allocator = _allocator;
    size = _size;
}

void VDepth::Create() {
    imageExtent = {(uint32_t)size.x,(uint32_t)size.y,1 };

    format = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo imageInfo = VInitializer::ImageInfo(format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, imageExtent);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr);

    VkImageViewCreateInfo imageViewInfo = VInitializer::ImageViewInfo(format, image, VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(device->rawDevice, &imageViewInfo, nullptr, &imageView));
}