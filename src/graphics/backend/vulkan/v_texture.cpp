#include <MARS/graphics/backend/vulkan/v_texture.hpp>
#include <MARS/graphics/backend/vulkan/v_buffer.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_image.hpp>
#include <MARS/graphics/backend/vulkan/v_type_helper.hpp>

using namespace mars_graphics;

VkImageView v_texture::raw_image_view() { return m_image->raw_image_view(); }

void v_texture::copy_buffer_to_image(v_buffer& buffer, VkImage _image) {
    VkCommandBuffer commandBuffer = cast_graphics<vulkan_backend>()->get_single_time_command();

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
        .imageExtent = { static_cast<uint32_t>(m_size.x), static_cast<uint32_t>(m_size.y), 1 },
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer.vulkan_buffer(), m_image->raw_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    cast_graphics<vulkan_backend>()->end_single_time_command(commandBuffer);
}

void v_texture::transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = cast_graphics<vulkan_backend>()->get_single_time_command();

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_image->raw_image(),
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
        mars_debug::debug::error("MARS - Vulkan - Texture - Unsupported layout transition!");

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    cast_graphics<vulkan_backend>()->end_single_time_command(commandBuffer);
}

void v_texture::create_sampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(cast_graphics<vulkan_backend>()->device()->raw_physical_device(), &properties);

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

    if (vkCreateSampler(cast_graphics<vulkan_backend>()->device()->raw_device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

bool v_texture::load_resource(const std::string &_texture_path) {
    if (!load_texture(_texture_path)) {
        free_og_texture();
        return false;
    }

    m_image = new v_image(cast_graphics<vulkan_backend>());

    m_image->set_format((VkFormat)(VK_FORMAT_R8_SRGB + m_channels * 7));
    m_image->set_size(m_size);
    m_image->set_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    v_buffer m_buffer(graphics());
    m_buffer.create(m_size.x * m_size.y * m_channels, MARS_MEMORY_TYPE_TRANSFER, 1);
    m_buffer.update(m_data);
    m_buffer.copy_data(0);

    m_image->create_image(VK_IMAGE_ASPECT_COLOR_BIT);

    transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(m_buffer, m_image->raw_image());
    transition_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, MARS2VK(m_layout));

    m_image->create_image_view();
    create_sampler();

    m_buffer.destroy();
    free_og_texture();
    return true;
}

void v_texture::create(MARS_FORMAT _format, MARS_TEXTURE_USAGE _usage) {
    m_format = _format;
    m_usage = _usage;

    VkImageAspectFlags aspectMask = 0;

    if (MARS2VK(_usage) & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    if (MARS2VK(_usage) & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (MARS2VK(m_format) >= VK_FORMAT_D16_UNORM_S8_UINT)
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    m_image = new v_image(cast_graphics<vulkan_backend>());
    m_image->set_format(MARS2VK(_format));
    m_image->set_usage(MARS2VK(_usage));
    m_image->set_size(m_size);
    m_image->create_image(aspectMask);
    m_image->create_image_view();
    create_sampler();
}

void v_texture::clean() {
    auto device = cast_graphics<vulkan_backend>()->device()->raw_device();
    vkDestroySampler(device, m_sampler, nullptr);

    m_image->destroy();
    delete m_image;
}