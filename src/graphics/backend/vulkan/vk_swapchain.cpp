#include "mars/graphics/backend/window.hpp"
#include "mars/graphics/window.hpp"
#include "mars/meta.hpp"
#include <mars/graphics/backend/vulkan/vk_swapchain.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>
#include <mars/graphics/backend/vulkan/vk_utils.hpp>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {

    namespace detail {
        sparse_vector<vk_swapchain, 5> swapchains;
        log_channel swapchain_channel("graphics/vulkan/swapchain");

        VkSurfaceFormatKHR choose_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& _available_formats) {
            for (const auto& available_format : _available_formats)
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return available_format;

            return _available_formats[0];
        }

        VkPresentModeKHR choose_swapchain_present_mode(const std::vector<VkPresentModeKHR>& _available_present_modes) {
            for (const auto& available_present_mode : _available_present_modes)
                if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return available_present_mode;

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& _capabilities, const window& _window) {
            if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
                return _capabilities.currentExtent;

            VkExtent2D actual_extent = {
                static_cast<uint32_t>(_window.size.x),
                static_cast<uint32_t>(_window.size.y)
            };

            actual_extent.width = std::clamp(actual_extent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(actual_extent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);

            return actual_extent;
        }

    } // namespace detail

    swapchain vk_swapchain_impl::vk_swapchain_create(const device& _device, const window& _window) {
        vk_swapchain* swapchain_ptr = detail::swapchains.request_entry();
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_window* window_ptr = static_cast<vk_window*>(_window.data);

        swapchain result;
        result.data = swapchain_ptr;
        result.engine = _device.engine;

        swapchain_support_details swapchain_support = query_swapchain_support(device_ptr->physical_device, window_ptr->surface);

        VkSurfaceFormatKHR surface_format = detail::choose_swapchain_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = detail::choose_swapchain_present_mode(swapchain_support.present_modes);
        VkExtent2D extent = detail::choose_swapchain_extent(swapchain_support.capabilities, _window);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;

        if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
            image_count = swapchain_support.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR create_info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = window_ptr->surface,
            .minImageCount = image_count,
            .imageFormat = surface_format.format,
            .imageColorSpace = surface_format.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = swapchain_support.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE
        };

        queue_family_indices indices = find_queue_families(device_ptr->physical_device, window_ptr->surface);
        uint32_t queue_family_indices[] = { indices.graphics_family, indices.present_family };

        if (indices.graphics_family != indices.present_family) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        } else
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult vk_result = vkCreateSwapchainKHR(device_ptr->device, &create_info, nullptr, &swapchain_ptr->swapchain);

        logger::assert_(vk_result == VK_SUCCESS, detail::swapchain_channel, "failed to create vk swapchain with error {}", meta::enum_to_string(vk_result));

        vkGetSwapchainImagesKHR(device_ptr->device, swapchain_ptr->swapchain, &image_count, nullptr);
        swapchain_ptr->swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device_ptr->device, swapchain_ptr->swapchain, &image_count, swapchain_ptr->swapchain_images.data());

        swapchain_ptr->swapchain_extent = extent;
        swapchain_ptr->swapchain_format = surface_format.format;

        // create images (maybe turn it into its own function in the future)
        swapchain_ptr->swapchain_images_views.resize(swapchain_ptr->swapchain_images.size());

        for (size_t i = 0; i < swapchain_ptr->swapchain_images_views.size(); i++) {
            VkImageViewCreateInfo image_create_info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapchain_ptr->swapchain_images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchain_ptr->swapchain_format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            vk_result = vkCreateImageView(device_ptr->device, &image_create_info, nullptr, &swapchain_ptr->swapchain_images_views[i]);
            logger::assert_(vk_result == VK_SUCCESS, detail::swapchain_channel, "failed to create vk swapchain image with error {}", meta::enum_to_string(vk_result));
        }
        return result;
    }

    void vk_swapchain_impl::vk_swapchain_destroy(swapchain& _swapchain, const device& _device) {
        vk_swapchain* swapchain_ptr = static_cast<vk_swapchain*>(_swapchain.data);
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);

        for (VkImageView image_view : swapchain_ptr->swapchain_images_views)
            vkDestroyImageView(device_ptr->device, image_view, nullptr);

        vkDestroySwapchainKHR(device_ptr->device, swapchain_ptr->swapchain, nullptr);

        detail::swapchains.remove(swapchain_ptr);
        _swapchain = {};
    }
} // namespace mars::graphics::vulkan