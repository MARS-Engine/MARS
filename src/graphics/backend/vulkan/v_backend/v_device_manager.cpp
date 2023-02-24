#include <MARS/graphics/backend/vulkan/v_backend/v_device_manager.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_instance.hpp>

using namespace mars_graphics;

std::vector<v_device*> mars_graphics::get_vulkan_devices(vulkan_backend* _backend_instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_backend_instance->instance()->raw_instance(), &deviceCount, nullptr);

    if (deviceCount == 0)
        mars_debug::debug::error("MARS - Vulkan - Failed to find GPU with Vulkan support");

    std::vector<VkPhysicalDevice> vulkan_devices(deviceCount);
    vkEnumeratePhysicalDevices(_backend_instance->instance()->raw_instance(), &deviceCount, vulkan_devices.data());

    std::vector<v_device*> valid_devices;

    for (const auto& physical_device : vulkan_devices) {
        auto new_device = new v_device(_backend_instance, physical_device);

        if (new_device->is_device_suitable())
            valid_devices.push_back(new_device);
        else
            delete new_device;
    }

    return valid_devices;
}