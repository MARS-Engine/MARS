#pragma once

#include <mars/graphics/backend/buffer_view.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_buffer_view {
        VkBufferView vk_buffer_view;
    };

    struct vk_buffer_view_impl {
        static buffer_view vk_buffer_view_create(const device& _device, const buffer_view_create_params& _params);
        static void vk_buffer_view_destroy(buffer_view& _buffer_view, const device& _device);
    };
} // namespace mars::graphics::vulkan