#pragma once

#include <mars/graphics/backend/instance.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/device.hpp>
#include <mars/graphics/window.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_swapchain {
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_images_views;

        VkFormat swapchain_format;
        VkExtent2D swapchain_extent;
    };

    struct vk_swapchain_impl {
        static swapchain vk_swapchain_create(const device& _device, const window& _window);
        static void vk_swapchain_destroy(swapchain& _swapchain, const device& _device);
    };
} // namespace mars::graphics::vulkan