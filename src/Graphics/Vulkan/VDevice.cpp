#include "VDevice.hpp"
#include "VSurface.hpp"
#include "VInstance.hpp"
#include "VkBootstrap.h"


VDevice::VDevice(VInstance* _instance, VSurface* _surface) {
    instance = _instance;
    surface = _surface;
}

void VDevice::Create() {

    vkb::PhysicalDeviceSelector selector{ instance->vkbInstance };
    vkb::PhysicalDevice vkbPhysicalDevice = selector.set_minimum_version(1, 1).set_surface(surface->rawSurface).select().value();
    vkb::DeviceBuilder deviceBuilder { vkbPhysicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();

    rawDevice = vkbDevice.device;
    physicalDevice = vkbPhysicalDevice.physical_device;

    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VDevice::Clean() const {
    vkDestroyDevice(rawDevice, nullptr);
}