#pragma once

#include <mars/graphics/backend/format.hpp>

#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {

    inline VkFormat mars_to_vk(mars_format_type _type) {
        switch (_type) {
        /* UNORM */
        case MARS_FORMAT_R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case MARS_FORMAT_RG8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case MARS_FORMAT_RGB8_UNORM:
            return VK_FORMAT_R8G8B8_UNORM;
        case MARS_FORMAT_RGBA8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        /* RGB SRGB */
        case MARS_FORMAT_R8_SRGB:
            return VK_FORMAT_R8_SRGB;
        case MARS_FORMAT_RG8_SRGB:
            return VK_FORMAT_R8G8_SRGB;
        case MARS_FORMAT_RGB8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case MARS_FORMAT_RGBA8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        /* BGR SRGB */
        case MARS_FORMAT_BGR8_SRGB:
            return VK_FORMAT_B8G8R8_SRGB;
        case MARS_FORMAT_BGRA8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        /* SFLOAT */
        case MARS_FORMAT_R32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case MARS_FORMAT_RG32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case MARS_FORMAT_RGB32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case MARS_FORMAT_RGBA32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        /* UINT */
        case MARS_FORMAT_R32_UINT:
            return VK_FORMAT_R32_UINT;
        case MARS_FORMAT_RG32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case MARS_FORMAT_RGB32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case MARS_FORMAT_RGBA32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        default:
            return VK_FORMAT_UNDEFINED;
        }
    }

    inline mars_format_type vk_to_mars(VkFormat _format) {
        switch (_format) {
        /* UNORM */
        case VK_FORMAT_R8_UNORM:
            return MARS_FORMAT_R8_UNORM;
        case VK_FORMAT_R8G8_UNORM:
            return MARS_FORMAT_RG8_UNORM;
        case VK_FORMAT_R8G8B8_UNORM:
            return MARS_FORMAT_RGB8_UNORM;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return MARS_FORMAT_RGBA8_UNORM;
        /* RGB SRGB */
        case VK_FORMAT_R8_SRGB:
            return MARS_FORMAT_R8_SRGB;
        case VK_FORMAT_R8G8_SRGB:
            return MARS_FORMAT_RG8_SRGB;
        case VK_FORMAT_R8G8B8_SRGB:
            return MARS_FORMAT_RGB8_SRGB;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return MARS_FORMAT_RGBA8_SRGB;
        /* BGR SRGB */
        case VK_FORMAT_B8G8R8_SRGB:
            return MARS_FORMAT_BGR8_SRGB;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return MARS_FORMAT_BGRA8_SRGB;
        /* SFLOAT */
        case VK_FORMAT_R32_SFLOAT:
            return MARS_FORMAT_R32_SFLOAT;
        case VK_FORMAT_R32G32_SFLOAT:
            return MARS_FORMAT_RG32_SFLOAT;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return MARS_FORMAT_RGB32_SFLOAT;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return MARS_FORMAT_RGBA32_SFLOAT;
        /* UINT */
        case VK_FORMAT_R32_UINT:
            return MARS_FORMAT_R32_UINT;
        case VK_FORMAT_R32G32_UINT:
            return MARS_FORMAT_RG32_UINT;
        case VK_FORMAT_R32G32B32_UINT:
            return MARS_FORMAT_RGB32_UINT;
        case VK_FORMAT_R32G32B32A32_UINT:
            return MARS_FORMAT_RGBA32_UINT;
        default:
            return MARS_FORMAT_UNDEFINED;
        }
    }

    inline VkImageUsageFlags mars_to_vk(mars_texture_usage _usage) {
        VkImageUsageFlags result = 0;

        if (_usage & MARS_TEXTURE_USAGE_TRANSFER_SRC)
            result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (_usage & MARS_TEXTURE_USAGE_TRANSFER_DST)
            result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (_usage & MARS_TEXTURE_USAGE_SAMPLED)
            result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (_usage & MARS_TEXTURE_USAGE_COLOR_ATTACHMENT)
            result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        return result;
    }

    inline unsigned char mars_format_to_channels(mars_format_type _type) {
        switch (_type) {
        case MARS_FORMAT_R8_SRGB:
        case MARS_FORMAT_R8_UNORM:
        case MARS_FORMAT_R32_SFLOAT:
        case MARS_FORMAT_R32_UINT:
            return 1;
        case MARS_FORMAT_RG8_SRGB:
        case MARS_FORMAT_RG8_UNORM:
        case MARS_FORMAT_RG32_SFLOAT:
        case MARS_FORMAT_RG32_UINT:
            return 2;
        case MARS_FORMAT_RGB8_SRGB:
        case MARS_FORMAT_RGB8_UNORM:
        case MARS_FORMAT_RGB32_SFLOAT:
        case MARS_FORMAT_RGB32_UINT:
            return 3;
        case MARS_FORMAT_RGBA8_SRGB:
        case MARS_FORMAT_RGBA8_UNORM:
        case MARS_FORMAT_RGBA32_SFLOAT:
        case MARS_FORMAT_RGBA32_UINT:
            return 4;
        default:
            return 0;
        }
    }

    inline unsigned char mars_format_size(mars_format_type _type) {
        switch (_type) {
        case MARS_FORMAT_R8_SRGB:
        case MARS_FORMAT_R8_UNORM:
        case MARS_FORMAT_RGB8_SRGB:
        case MARS_FORMAT_RGB8_UNORM:
        case MARS_FORMAT_RGBA8_SRGB:
        case MARS_FORMAT_RGBA8_UNORM:
            return 1;
        case MARS_FORMAT_RG8_SRGB:
        case MARS_FORMAT_RG8_UNORM:
        case MARS_FORMAT_R32_SFLOAT:
        case MARS_FORMAT_R32_UINT:
        case MARS_FORMAT_RG32_SFLOAT:
        case MARS_FORMAT_RG32_UINT:
        case MARS_FORMAT_RGB32_SFLOAT:
        case MARS_FORMAT_RGB32_UINT:
        case MARS_FORMAT_RGBA32_SFLOAT:
        case MARS_FORMAT_RGBA32_UINT:
            return 4;
        default:
            return 0;
        }
    }

    inline VkBufferUsageFlags mars_buffer_usage_to_vulkan(uint32_t _type) {
        VkBufferUsageFlags result = 0;

        if (_type & MARS_BUFFER_TYPE_VERTEX)
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (_type & MARS_BUFFER_TYPE_INDEX)
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (_type & MARS_BUFFER_TYPE_UNIFORM_TEXEL_BUFFER)
            result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        if (_type & MARS_BUFFER_TYPE_UNIFORM)
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (_type & MARS_BUFFER_TYPE_TRANSFER_DST)
            result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        if (_type & MARS_BUFFER_TYPE_TRANSFER_SRC)
            result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        return result;
    }

    inline VkBufferUsageFlags mars_buffer_properties_to_vulkan(uint32_t _property) {
        VkBufferUsageFlags result = 0;
        if (_property & MARS_BUFFER_PROPERTY_HOST_VISIBLE)
            result |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        if (_property & MARS_BUFFER_PROPERTY_DEVICE_LOCAL)
            result |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        return result;
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