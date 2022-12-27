#include <MVRE/graphics/backend/vulkan/v_backend/v_image.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mvre_graphics;

void v_image::create_image() {
    VkImageCreateInfo image_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_format,
            .extent = {
                    .width = m_size.x(),
                    .height = m_size.y(),
                    .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = m_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    auto device = graphics_instance()->device()->raw_device();

    if (vkCreateImage(device, &image_info, nullptr, &m_image) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);

    VkMemoryAllocateInfo alloc_info {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = graphics_instance()->device()->find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    if (vkAllocateMemory(device, &alloc_info, nullptr, &m_image_memory) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Failed to allocate image memory");

    vkBindImageMemory(device, m_image, m_image_memory, 0);
}

void v_image::create_image_view() {
    VkImageViewCreateInfo view_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_format,
            .subresourceRange = {
                    .aspectMask = m_aspect_flag,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            }
    };

    if (vkCreateImageView(graphics_instance()->device()->raw_device(), &view_info, nullptr, &m_image_view) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Failed to create image view");
}

void v_image::destroy() {
    vkDestroyImageView(graphics_instance()->device()->raw_device(), m_image_view, nullptr);
    vkDestroyImage(graphics_instance()->device()->raw_device(), m_image, nullptr);
    vkFreeMemory(graphics_instance()->device()->raw_device(), m_image_memory, nullptr);
}