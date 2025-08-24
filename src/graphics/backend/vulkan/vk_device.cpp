
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_device.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>
#include <mars/meta.hpp>

#include <set>
#include <vector>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_device, 2> devices;
        log_channel device_channel("graphics/vulkan/device");

        queue_family_indices find_queue_families(VkPhysicalDevice _device, VkSurfaceKHR _surface) {
            queue_family_indices indices;

            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(_device, &queue_family_count, nullptr);

            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(_device, &queue_family_count, queue_families.data());

            for (int i = 0; i < queue_family_count; i++) {
                if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.graphics_family = i;

                VkBool32 present_support = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, _surface, &present_support);

                if (present_support)
                    indices.present_family = i;

                if (indices.is_valid())
                    break;
            }

            return indices;
        }

        bool is_device_suitable(VkPhysicalDevice _device, VkSurfaceKHR _surface) {
            if (!find_queue_families(_device, _surface).is_valid())
                return false;

            VkPhysicalDeviceProperties device_properties;
            VkPhysicalDeviceFeatures device_features;
            vkGetPhysicalDeviceProperties(_device, &device_properties);
            vkGetPhysicalDeviceFeatures(_device, &device_features);

            return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader;
        }
    }; // namespace detail

    device vk_device_impl::vk_device_create(instance& _instace, window& _window) {
        device result;

        vk_instance* instance_ptr = static_cast<vk_instance*>(_instace.data);
        vk_window* window_ptr = static_cast<vk_window*>(_window.data);

        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_ptr->instance, &device_count, nullptr);

        logger::assert_(device_count, detail::device_channel, "");

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(instance_ptr->instance, &device_count, physical_devices.data());

        VkPhysicalDevice physical_device = VK_NULL_HANDLE;

        for (const auto& device : physical_devices) {
            if (detail::is_device_suitable(device, window_ptr->surface)) {
                physical_device = device;
                break;
            }
        }

        logger::assert_(physical_device != VK_NULL_HANDLE, detail::device_channel, "");

        queue_family_indices indices = detail::find_queue_families(physical_device, window_ptr->surface);

        float queue_priority = 1.0f;

        VkPhysicalDeviceFeatures device_features{};

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphics_family, indices.present_family };

        for (uint32_t queue_family : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queue_create_info{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            };
            queue_create_infos.push_back(queue_create_info);
        }

        VkDeviceCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledLayerCount = 0,
            .enabledExtensionCount = 0,
            .pEnabledFeatures = &device_features,
        };

        vk_device* device_ptr = detail::devices.request_entry();

        device_ptr->debug_mode = _instace.debug_mode;

        if (device_ptr->debug_mode) {
            create_info.enabledLayerCount = static_cast<uint32_t>(instance_ptr->instance_layers.size());
            create_info.ppEnabledLayerNames = instance_ptr->instance_layers.data();
        }

        VkResult vk_result = vkCreateDevice(physical_device, &create_info, nullptr, &device_ptr->device);

        logger::assert_(vk_result == VK_SUCCESS, detail::device_channel, "failed to create vulkan device with error {}", meta::enum_to_string(vk_result));

        vkGetDeviceQueue(device_ptr->device, indices.graphics_family, 0, &device_ptr->graphics_queue);
        vkGetDeviceQueue(device_ptr->device, indices.present_family, 0, &device_ptr->present_queue);

        result.engine = _instace.engine;
        result.data = device_ptr;
        return result;
    }

    void vk_device_impl::vk_device_destroy(device& _device) {
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vkDestroyDevice(device_ptr->device, nullptr);
        detail::devices.remove(device_ptr);
    }
} // namespace mars::graphics::vulkan