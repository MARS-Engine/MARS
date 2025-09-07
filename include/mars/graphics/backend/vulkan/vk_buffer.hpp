#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/command_pool.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_buffer {
        VkBuffer vk_buffer;
        VkDeviceMemory device_memory;
    };

    struct vk_buffer_impl {
        static buffer vk_buffer_create(const device& _device, const buffer_create_params& _params);
        static void vk_buffer_bind(buffer& _buffer, const command_buffer& _command_buffer);
        static void vk_buffer_bind_index(buffer& _buffer, const command_buffer& _command_buffer);
        static void* vk_buffer_map(buffer& _buffer, const device& _device, size_t _size, size_t _offset);
        static void vk_buffer_unmap(buffer& _buffer, const device& _device);
        static void vk_buffer_destroy(buffer& _buffer, const device& _device);
    };
} // namespace mars::graphics::vulkan