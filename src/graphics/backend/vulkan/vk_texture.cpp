#include "mars/graphics/backend/command_pool.hpp"
#include "mars/graphics/backend/format.hpp"
#include "mars/meta.hpp"
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_texture.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_buffer.hpp>
#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_utils.hpp>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_texture, 24> textures;
        log_channel texture_channel("graphics/vulkan/texture");

        void transition_image_layout(VkCommandBuffer _command_buffer, VkImage _image, VkImageLayout _old_layout, VkImageLayout _new_layout) {
            VkImageMemoryBarrier barrier{
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = _old_layout,
                .newLayout = _new_layout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = _image,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            VkPipelineStageFlags source_stage;
            VkPipelineStageFlags destination_stage;

            if (_old_layout == VK_IMAGE_LAYOUT_UNDEFINED && _new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (_old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else
                logger::assert_(texture_channel, "unsupported layout transition from {} to {}", meta::enum_to_string(_old_layout), meta::enum_to_string(_new_layout));

            vkCmdPipelineBarrier(_command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }

        void vk_extract_texture_view(const texture_view& _view, std::vector<VkImageView>& _out) {
            vk_texture* texture_ptr = _view.data;

            _out.push_back(texture_ptr->view);
        }
    } // namespace detail

    texture vk_texture_impl::vk_texture_create(const device& _device, const texture_create_params& _params) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_texture* texture_ptr = detail::textures.request_entry();

        texture result;
        result.engine = _device.engine;
        result.data = texture_ptr;
        result.size = _params.size;
        result.channels = mars_format_to_channels(_params.format);
        result.format_size = mars_format_size(_params.format);
        result.view.data = texture_ptr;
        result.view.view_extractor = &detail::vk_extract_texture_view;

        VkImageCreateInfo image_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = mars_to_vk(_params.format),
            .extent = {
                .width = static_cast<uint32_t>(_params.size.x),
                .height = static_cast<uint32_t>(_params.size.y),
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = static_cast<uint32_t>(_params.array_size),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = mars_to_vk(_params.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VkResult vk_result = vkCreateImage(device_ptr->device, &image_info, nullptr, &texture_ptr->image);

        logger::assert_(vk_result == VK_SUCCESS, detail::texture_channel, "failed to create image with error {}", meta::enum_to_string(vk_result));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device_ptr->device, texture_ptr->image, &memRequirements);

        VkMemoryAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = find_memory_type(device_ptr->physical_device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
        };

        logger::assert_(alloc_info.memoryTypeIndex != -1, detail::texture_channel, "failed to find a valid memory type for texture");

        vk_result = vkAllocateMemory(device_ptr->device, &alloc_info, nullptr, &texture_ptr->memory);

        logger::assert_(vk_result == VK_SUCCESS, detail::texture_channel, "failed to allocate image memory with error {}", meta::enum_to_string(vk_result));

        vk_result = vkBindImageMemory(device_ptr->device, texture_ptr->image, texture_ptr->memory, 0);

        logger::assert_(vk_result == VK_SUCCESS, detail::texture_channel, "failed to bind image to memory with error {}", meta::enum_to_string(vk_result));

        VkImageViewCreateInfo view_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture_ptr->image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = image_info.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk_result = vkCreateImageView(device_ptr->device, &view_info, nullptr, &texture_ptr->view);

        logger::assert_(vk_result == VK_SUCCESS, detail::texture_channel, "failed to create a image view with error {}", meta::enum_to_string(vk_result));

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device_ptr->physical_device, &properties);

        VkSamplerCreateInfo sampler_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = _params.filter == MARS_TEXTURE_FILTER_LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
            .minFilter = _params.filter == MARS_TEXTURE_FILTER_LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST,
            .mipmapMode = _params.filter == MARS_TEXTURE_FILTER_LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST,
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

        vk_result = vkCreateSampler(device_ptr->device, &sampler_info, nullptr, &texture_ptr->sampler);

        logger::assert_(vk_result == VK_SUCCESS, detail::texture_channel, "failed to create a sampler with error {}", meta::enum_to_string(vk_result));

        return result;
    }

    void vk_texture_impl::vk_texture_copy(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) {
        vk_texture* texture_ptr = _texture.data.get<vk_texture>();
        vk_buffer* buffer_ptr = _src_buffer.data.get<vk_buffer>();
        vk_command_pool* command_ptr = _command_buffer.data.get<vk_command_pool>();

        detail::transition_image_layout(command_ptr->command_buffers[_command_buffer.buffer_index], texture_ptr->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region{
            .bufferOffset = _offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = {
                static_cast<uint32_t>(_texture.size.x),
                static_cast<uint32_t>(_texture.size.y),
                1,
            },
        };

        vkCmdCopyBufferToImage(command_ptr->command_buffers[_command_buffer.buffer_index], buffer_ptr->vk_buffer, texture_ptr->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        detail::transition_image_layout(command_ptr->command_buffers[_command_buffer.buffer_index], texture_ptr->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void vk_texture_impl::vk_texture_destroy(texture& _texture, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_texture* texture_ptr = _texture.data.get<vk_texture>();

        vkDestroySampler(device_ptr->device, texture_ptr->sampler, nullptr);
        vkDestroyImageView(device_ptr->device, texture_ptr->view, nullptr);
        vkDestroyImage(device_ptr->device, texture_ptr->image, nullptr);
        vkFreeMemory(device_ptr->device, texture_ptr->memory, nullptr);

        detail::textures.remove(texture_ptr);
        _texture = {};
    }
} // namespace mars::graphics::vulkan