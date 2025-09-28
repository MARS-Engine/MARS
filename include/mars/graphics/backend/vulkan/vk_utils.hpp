#pragma once

#include <mars/graphics/backend/format.hpp>

#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    inline VkFormat mars_format_to_vk(const format& _format) {
        switch (_format.type) {
        case MARS_FORMAT_TYPE_SFLOAT:
            if (_format.size == 32) {
                if (_format.number == 1)
                    return VK_FORMAT_R32_SFLOAT;
                if (_format.number == 2)
                    return VK_FORMAT_R32G32_SFLOAT;
                if (_format.number == 3)
                    return VK_FORMAT_R32G32B32_SFLOAT;
            }
            break;
        case MARS_FORMAT_TYPE_UINT:
            if (_format.size == 32) {
                if (_format.number == 1)
                    return VK_FORMAT_R32_UINT;
                if (_format.number == 2)
                    return VK_FORMAT_R32G32_UINT;
                if (_format.number == 3)
                    return VK_FORMAT_R32G32B32_UINT;
                if (_format.number == 4)
                    return VK_FORMAT_R32G32B32A32_UINT;
            }
            break;
        }

        return VK_FORMAT_UNDEFINED;
    }

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

    inline uint32_t find_memory_type(VkPhysicalDevice _device, uint32_t _type_filter, VkMemoryPropertyFlags _properties) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(_device, &memory_properties);

        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
            if ((_type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & _properties) == _properties)
                return i;

        return -1;
    }
} // namespace mars::graphics::vulkan