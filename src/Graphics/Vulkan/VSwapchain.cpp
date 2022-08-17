#include "VSwapchain.hpp"
#include "VDevice.hpp"
#include "VSurface.hpp"
#include "../Window.hpp"
#include "VkBootstrap.h"

VSwapchain::VSwapchain(VDevice* _device) {
    device = _device;
}

void VSwapchain::Create() {
    //No swapchain size will ever be bigger than a float
    size = Vector2(device->surface->window->windowSize.width, device->surface->window->windowSize.height);

    vkb::SwapchainBuilder swapchainBuilder { device->physicalDevice, device->rawDevice, device->surface->rawSurface  };
    vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR).set_desired_extent(size.x, size.y).build().value();
    //Must use VK_PRESENT_MODE_MAILBOX_KHR or any other non-tearing updater otherwise program crash because of simultaneous use

    rawSwapchain = vkbSwapchain.swapchain;
    images = vkbSwapchain.get_images().value();
    imageViews = vkbSwapchain.get_image_views().value();
    format = vkbSwapchain.image_format;
}

void VSwapchain::Clean() {
    vkDestroySwapchainKHR(device->rawDevice, rawSwapchain, nullptr);

    for (auto& imageView : imageViews)
        vkDestroyImageView(device->rawDevice, imageView, nullptr);
}