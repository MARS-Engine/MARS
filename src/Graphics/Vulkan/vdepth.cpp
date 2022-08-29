#include "vdepth.hpp"
#include "vswapchain.hpp"
#include "vdevice.hpp"

vdepth::vdepth(VmaAllocator& _allocator, vdevice* _device, vector2 _size) {
    device = _device;
    allocator = _allocator;
    size = _size;
}

void vdepth::create() {
    image_extent = {(uint32_t)size.x, (uint32_t)size.y, 1 };

    format = VK_FORMAT_D32_SFLOAT;

    VkImageCreateInfo imageInfo = VInitializer::image_info(format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                           image_extent);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr));

    VkImageViewCreateInfo imageViewInfo = VInitializer::image_view_info(format, image, VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(device->raw_device, &imageViewInfo, nullptr, &image_view));
}