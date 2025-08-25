#include "mars/graphics/backend/window.hpp"
#include <mars/graphics/backend/vulkan/vk_device.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>
#include <mars/graphics/backend/vulkan/vk_utils.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/meta.hpp>

#include <cstdint>
#include <set>
#include <vector>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_device, 2> devices;
        log_channel device_channel("graphics/vulkan/device");

        bool has_extensions(VkPhysicalDevice _device, const std::vector<const char*>& _extensions) {
            uint32_t extension_count;
            vkEnumerateDeviceExtensionProperties(_device, nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> available_extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(_device, nullptr, &extension_count, available_extensions.data());

            std::set<std::string> result;
            result.insert_range(_extensions);

            for (const auto& extension : available_extensions)
                result.erase(extension.extensionName);

            return result.empty();
        }

        bool is_device_suitable(VkPhysicalDevice _device, VkSurfaceKHR _surface, const std::vector<const char*>& _required_extensions) {
            if (!find_queue_families(_device, _surface).is_valid())
                return false;

            if (!has_extensions(_device, _required_extensions))
                return false;

            swapchain_support_details swapchain_support = query_swapchain_support(_device, _surface);
            if (swapchain_support.formats.empty() || swapchain_support.present_modes.empty())
                return false;

            return true;
        }
    }; // namespace detail

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

    device vk_device_impl::vk_device_create(instance& _instace, window& _window) {
        device result;

        vk_instance* instance_ptr = static_cast<vk_instance*>(_instace.data);
        vk_window* window_ptr = static_cast<vk_window*>(_window.data);

        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_ptr->instance, &device_count, nullptr);

        logger::assert_(device_count, detail::device_channel, "");

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(instance_ptr->instance, &device_count, physical_devices.data());

        std::vector<const char*> required_extensions;

        _window.engine->get_impl<window_impl>().window_get_device_extensions(_window, required_extensions);

        vk_device* device_ptr = detail::devices.request_entry();

        for (const auto& device : physical_devices) {
            if (detail::is_device_suitable(device, window_ptr->surface, required_extensions)) {
                device_ptr->physical_device = device;
                break;
            }
        }

        logger::assert_(device_ptr->physical_device != VK_NULL_HANDLE, detail::device_channel, "");

        queue_family_indices indices = find_queue_families(device_ptr->physical_device, window_ptr->surface);

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
            .enabledExtensionCount = static_cast<uint32_t>(required_extensions.size()),
            .ppEnabledExtensionNames = required_extensions.data(),
            .pEnabledFeatures = &device_features,
        };

        device_ptr->debug_mode = _instace.debug_mode;

        if (device_ptr->debug_mode) {
            create_info.enabledLayerCount = static_cast<uint32_t>(instance_ptr->instance_layers.size());
            create_info.ppEnabledLayerNames = instance_ptr->instance_layers.data();
        }

        VkResult vk_result = vkCreateDevice(device_ptr->physical_device, &create_info, nullptr, &device_ptr->device);

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
        _device = {};
    }
} // namespace mars::graphics::vulkan