#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {
    struct swapchain_support_details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    inline swapchain_support_details query_swapchain_support(VkPhysicalDevice _device, VkSurfaceKHR _surface) {
        swapchain_support_details details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, _surface, &details.capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &format_count, nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &format_count, details.formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &present_mode_count, details.present_modes.data());
        }

        return details;
    }
} // namespace mars::graphics::vulkan