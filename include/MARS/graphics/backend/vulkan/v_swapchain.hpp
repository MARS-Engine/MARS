#ifndef MARS_V_SWAPCHAIN_
#define MARS_V_SWAPCHAIN_

#include "MARS/graphics/backend/template/swapchain.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace mars_graphics {

    class v_swapchain : public swapchain {
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

        [[nodiscard]] inline std::vector<VkImageView>& image_views() { return m_swapchain_image_views; }
        [[nodiscard]] inline VkSwapchainKHR&  raw_swapchain() { return m_swapchain; }
        [[nodiscard]] inline VkFormat image_format() const { return  m_swapchain_image_format; }
        [[nodiscard]] inline VkExtent2D extent() const { return  m_swapchain_extent; }

        using swapchain::swapchain;

        void create() override;

        void destroy() override;
    };
}

#endif