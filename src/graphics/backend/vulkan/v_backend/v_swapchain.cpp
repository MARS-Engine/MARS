#include <MVRE/graphics/backend/vulkan/v_backend/v_swapchain.hpp>
#include <SDL2/SDL_vulkan.h>


using namespace mvre_graphics;

VkSurfaceFormatKHR v_swapchain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& _available_formats) {
    for (const auto& available_format : _available_formats)
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return available_format;
    return _available_formats[0];
}
VkPresentModeKHR v_swapchain::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& _available_present_modes) {
    for (const auto& available_present_mode : _available_present_modes)
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            return available_present_mode;

    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D v_swapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    else {
        int width, height;
        SDL_Vulkan_GetDrawableSize(graphics_instance()->get_vulkan_window()->raw_window(), &width, &height);

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void v_swapchain::create() {
    auto swapchain_support = graphics_instance()->device()->query_swap_chain_support();

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR swap_create_info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = graphics_instance()->get_vulkan_window()->raw_surface(),
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

    auto indices = graphics_instance()->device()->family_indices();
    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        swap_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_create_info.queueFamilyIndexCount = 2;
        swap_create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
        swap_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateSwapchainKHR(graphics_instance()->device()->raw_device(), &swap_create_info, nullptr, &m_swapchain))
        mvre_debug::debug::error("MVRE - Vulkan - Failed to create v_swapchain");

    vkGetSwapchainImagesKHR(graphics_instance()->device()->raw_device(), m_swapchain, &image_count, nullptr);
    m_swapchain_images.resize(image_count);
    vkGetSwapchainImagesKHR(graphics_instance()->device()->raw_device(), m_swapchain, &image_count, m_swapchain_images.data());

    m_swapchain_image_format = surface_format.format;
    m_swapchain_extent = extent;

    m_swapchain_image_views.reserve(image_count);

    for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
        VkImageViewCreateInfo view_create_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_swapchain_image_format,
            .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
            }
        };

        if (vkCreateImageView(graphics_instance()->device()->raw_device(), &view_create_info, nullptr, &m_swapchain_image_views[i]))
            mvre_debug::debug::error("MVRE - Vulkan - Failed to create image views");
    }
}