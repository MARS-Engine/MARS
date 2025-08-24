#pragma once

#include <mars/graphics/backend/instance.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/window.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_swapchain {
    };

    struct vk_swapchain_impl {
        static swapchain vk_swapchain_create(instance& _instance, window& _window);
        static void vk_swapchain_destroy(swapchain& _swapchain, instance& _instance);
    };
} // namespace mars::graphics::vulkan