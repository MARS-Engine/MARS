#include <MVRE/graphics/backend/vulkan/v_texture.hpp>
#include <MVRE/graphics/backend/vulkan/v_buffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>

using namespace mvre_graphics;

void v_texture::copy_buffer_to_image(v_buffer& buffer, VkImage _image) {
    VkCommandBuffer commandBuffer = instance<v_backend_instance>()->get_single_time_command();

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
        .imageOffset = {0, 0, 0},
        .imageExtent = { static_cast<uint32_t>(m_size.x()), static_cast<uint32_t>(m_size.y()), 1 },
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer.vulkan_buffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    instance<v_backend_instance>()->end_single_time_command(commandBuffer);
}

void v_texture::transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = instance<v_backend_instance>()->get_single_time_command();

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Unsupported layout transition!");

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    instance<v_backend_instance>()->end_single_time_command(commandBuffer);
}

void v_texture::create_image() {
    VkImageCreateInfo image_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_format,
        .extent = {
            .width = static_cast<uint32_t>(m_size.x()),
            .height = static_cast<uint32_t>(m_size.y()),
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    auto device = instance<v_backend_instance>()->device()->raw_device();

    if (vkCreateImage(device, &image_info, nullptr, &m_image) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, m_image, &memRequirements);

    VkMemoryAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = instance<v_backend_instance>()->device()->find_memory_type(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };

    if (vkAllocateMemory(device, &alloc_info, nullptr, &m_image_memory) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Failed to allocate image memory");

    vkBindImageMemory(device, m_image, m_image_memory, 0);
}

void v_texture::create_image_view(VkFormat _format) {
    VkImageViewCreateInfo view_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = _format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
    };

    if (vkCreateImageView(instance<v_backend_instance>()->device()->raw_device(), &view_info, nullptr, &m_image_view) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Texture - Failed to create image view");
}

void v_texture::create_sampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(instance<v_backend_instance>()->device()->raw_physical_device(), &properties);

    VkSamplerCreateInfo samplerInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    if (vkCreateSampler(instance<v_backend_instance>()->device()->raw_device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

bool v_texture::load_resource(const std::string &_texture_path) {
    if (!load_texture(_texture_path)) {
        free_og_texture();
        return false;
    }

    m_format = (VkFormat)(VK_FORMAT_R8_SRGB + m_channels * 7);

    v_buffer m_buffer(instance());
    m_buffer.create(m_size.x() * m_size.y() * m_channels, MVRE_MEMORY_TYPE_TRANSFER);
    m_buffer.copy_data(m_data);

    create_image();

    transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(m_buffer, m_image);
    transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    create_image_view(m_format);
    create_sampler();

    m_buffer.destroy();
    free_og_texture();
    return true;
}

void v_texture::clean() {
    auto device = instance<v_backend_instance>()->device()->raw_device();
    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyImageView(device, m_image_view, nullptr);
    vkDestroyImage(device, m_image, nullptr);
    vkFreeMemory(device, m_image_memory, nullptr);
}