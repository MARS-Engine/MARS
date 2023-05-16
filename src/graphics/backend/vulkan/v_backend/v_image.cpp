#include <MARS/graphics/backend/vulkan/v_backend/v_image.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mars_graphics;

void v_image::create_image(VkImageAspectFlags _aspect_flag) {
    m_aspect_flag = _aspect_flag;

    VkImageCreateInfo image_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_format,
            .extent = {
                    .width = m_size.x,
                    .height = m_size.y,
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

    auto device = graphics()->get_device()->raw_device();

    if (vkCreateImage(device, &image_info, nullptr, &m_image) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Texture - Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);

    VkMemoryAllocateInfo alloc_info {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = graphics()->get_device()->find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    if (vkAllocateMemory(device, &alloc_info, nullptr, &m_image_memory) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Texture - Failed to allocate image memory");

    vkBindImageMemory(device, m_image, m_image_memory, 0);
}

void v_image::copy_buffer_to_image(v_buffer* buffer, const mars_math::vector4<uint32_t>& _rect) {
    VkCommandBuffer commandBuffer = graphics()->get_single_time_command();

    VkBufferImageCopy region {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            },
            .imageOffset = { static_cast<int32_t>(_rect.x) , static_cast<int32_t>(_rect.y), 0},
            .imageExtent = { _rect.z, _rect.w, 1 },
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer->vulkan_buffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    graphics()->end_single_time_command(commandBuffer);
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

    if (vkCreateImageView(graphics()->get_device()->raw_device(), &view_info, nullptr, &m_image_view) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Texture - Failed to create image view");
}

void v_image::destroy() {
    vkDestroyImageView(graphics()->get_device()->raw_device(), m_image_view, nullptr);
    vkDestroyImage(graphics()->get_device()->raw_device(), m_image, nullptr);
    vkFreeMemory(graphics()->get_device()->raw_device(), m_image_memory, nullptr);
}