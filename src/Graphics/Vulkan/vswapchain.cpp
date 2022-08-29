#include "vswapchain.hpp"
#include "vdevice.hpp"
#include "vsurface.hpp"
#include "../window.hpp"
#include "VkBootstrap.h"

vswapchain::vswapchain(vdevice* _device) {
    device = _device;
}

void vswapchain::create() {
    //No swapchain size will ever be bigger than a float
    size = vector2(device->surface->surf_window->window_size.width, device->surface->surf_window->window_size.height);

    vkb::SwapchainBuilder swapchainBuilder {device->physical_device, device->raw_device, device->surface->raw_surface  };
    vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR).set_desired_extent(size.x, size.y).build().value();
    //Must use VK_PRESENT_MODE_MAILBOX_KHR or any other non-tearing updater otherwise program crash because of simultaneous use

    raw_swapchain = vkbSwapchain.swapchain;
    images = vkbSwapchain.get_images().value();
    image_views = vkbSwapchain.get_image_views().value();
    format = vkbSwapchain.image_format;
}

void vswapchain::clean() {
    vkDestroySwapchainKHR(device->raw_device, raw_swapchain, nullptr);

    for (auto& imageView : image_views)
        vkDestroyImageView(device->raw_device, imageView, nullptr);
}