#ifndef MVRE_V_SWAPCHAIN_
#define MVRE_V_SWAPCHAIN_

#include <MVRE/graphics/backend/vulkan/v_backend/v_device.hpp>
#include "v_base.hpp"
#include <vector>

namespace mvre_graphics {

    class v_swapchain : v_base {
    private:
        std::vector<VkImageView> m_swapchain_image_views;
        std::vector<VkImage> m_swapchain_images;
        VkSwapchainKHR m_swapchain;
        VkFormat m_swapchain_image_format;
        VkExtent2D m_swapchain_extent;

        VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& _available_formats);
        VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& _available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
    public:

        inline std::vector<VkImageView>& image_views() { return m_swapchain_image_views; }
        inline VkSwapchainKHR&  raw_swapchain() { return m_swapchain; }
        inline VkFormat image_format() const { return  m_swapchain_image_format; }
        inline VkExtent2D extent() const { return  m_swapchain_extent; }

        using v_base::v_base;

        void create();

        inline void destroy() {
            for (auto image_view : m_swapchain_image_views)
                vkDestroyImageView(graphics_instance()->device()->raw_device(), image_view, nullptr);
            vkDestroySwapchainKHR(graphics_instance()->device()->raw_device(), m_swapchain, nullptr);
        }
    };
}

#endif