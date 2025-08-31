#pragma once

#include <mars/graphics/backend/command_pool.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_command_pool {
        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;
    };

    struct vk_command_pool_impl {
        static command_pool vk_command_pool_create(const device& _device);
        static std::vector<command_buffer> vk_command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _n_command_buffers);
        static void vk_command_buffer_reset(const command_buffer& _command_buffer);
        static void vk_command_buffer_record(const command_buffer& _command_buffer);
        static void vk_command_buffer_record_end(const command_buffer& _command_buffer);
        static void vk_command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params);
        static void vk_command_pool_destroy(command_pool& _command_pool, const device& _device);
    };
} // namespace mars::graphics::vulkan