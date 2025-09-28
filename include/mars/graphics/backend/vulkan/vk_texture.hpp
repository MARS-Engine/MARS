#pragma once

#include <mars/graphics/backend/texture.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_texture {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
        VkSampler sampler;
    };

    struct vk_texture_impl {
        static texture vk_texture_create(const device& _device, const texture_create_params& _params);
        static void vk_texture_copy(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset);
        static void vk_texture_destroy(texture& _texture, const device& _device);
    };
} // namespace mars::graphics::vulkan