#include "vdevice.hpp"
#include "vsurface.hpp"
#include "vinstance.hpp"
#include "VkBootstrap.h"


vdevice::vdevice(vinstance* _instance, vsurface* _surface) {
    instance = _instance;
    surface = _surface;
}

void vdevice::create() {

    vkb::PhysicalDeviceSelector selector{ instance->vkb_instance };
    vkb::PhysicalDevice vkbPhysicalDevice = selector.set_minimum_version(1, 1).set_surface(surface->raw_surface).select().value();
    vkb::DeviceBuilder deviceBuilder { vkbPhysicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    raw_device = vkbDevice.device;
    physical_device = vkbPhysicalDevice.physical_device;

    graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void vdevice::clean() const {
    vkDestroyDevice(raw_device, nullptr);
}