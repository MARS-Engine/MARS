#pragma once

#include <mars/graphics/backend/device.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct queue_family_indices {
        uint32_t graphics_family = -1;
        uint32_t present_family = -1;

        inline bool is_valid() const { return graphics_family != -1 && present_family != -1; }
    };

    struct vk_device {
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphics_queue = VK_NULL_HANDLE;
        VkQueue present_queue = VK_NULL_HANDLE;
        bool debug_mode = false;
    };

    struct vk_device_impl {
        static device vk_device_create(instance& _instace, window& _window);
        static void vk_device_destroy(device& _device);
    };
} // namespace mars::graphics::vulkan