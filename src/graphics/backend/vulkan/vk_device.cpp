#include "mars/graphics/backend/device.hpp"
#include "mars/graphics/backend/vulkan/vk_swapchain.hpp"
#include "mars/graphics/backend/vulkan/vk_sync.hpp"
#include <mars/graphics/backend/vulkan/vk_device.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>
#include <mars/graphics/backend/vulkan/vk_utils.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/meta.hpp>

#include <cstdint>
#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>

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

    device vk_device_impl::vk_device_create(instance& _instance, window& _window) {
        device result;

        vk_instance* instance_ptr = _instance.data.get<vk_instance>();
        vk_window* window_ptr = _window.data.get<vk_window>();

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

        device_ptr->queue_indices = find_queue_families(device_ptr->physical_device, window_ptr->surface);

        float queue_priority = 1.0f;

        VkPhysicalDeviceFeatures device_features{};

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> uniqueQueueFamilies = { device_ptr->queue_indices.graphics_family, device_ptr->queue_indices.present_family };

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

        device_ptr->debug_mode = _instance.debug_mode;

        if (device_ptr->debug_mode) {
            create_info.enabledLayerCount = static_cast<uint32_t>(instance_ptr->instance_layers.size());
            create_info.ppEnabledLayerNames = instance_ptr->instance_layers.data();
        }

        VkResult vk_result = vkCreateDevice(device_ptr->physical_device, &create_info, nullptr, &device_ptr->device);

        logger::assert_(vk_result == VK_SUCCESS, detail::device_channel, "failed to create vulkan device with error {}", meta::enum_to_string(vk_result));

        vkGetDeviceQueue(device_ptr->device, device_ptr->queue_indices.graphics_family, 0, &device_ptr->graphics_queue);
        vkGetDeviceQueue(device_ptr->device, device_ptr->queue_indices.present_family, 0, &device_ptr->present_queue);

        result.engine = _instance.engine;
        result.data = device_ptr;
        return result;
    }

    void vk_device_impl::vk_device_submit_graphics_queue(const device& _device, const sync& _sync, size_t _current_index, size_t _image_index, const command_buffer* _buffers, size_t _n_buffers) {
        if (_n_buffers == 0)
            return logger::error(detail::device_channel, "attempted to submit 0 command buffers to queue");

        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_sync* sync_ptr = _sync.data.get<vk_sync>();
        vk_command_pool* command_pool_ptr = _buffers[0].data.get<vk_command_pool>();

        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        std::vector<VkCommandBuffer> buffers;
        buffers.resize(_n_buffers);

        for (size_t i = 0; i < _n_buffers; i++)
            buffers[i] = command_pool_ptr->command_buffers[_buffers[i].buffer_index];

        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &sync_ptr->image_available_semaphore[_current_index],
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = static_cast<uint32_t>(_n_buffers),
            .pCommandBuffers = buffers.data(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &sync_ptr->render_finished_semaphore[_image_index],
        };

        VkResult vk_result = vkQueueSubmit(device_ptr->graphics_queue, 1, &submit_info, sync_ptr->in_flight_fence[_current_index]);

        logger::assert_(vk_result == VK_SUCCESS, detail::device_channel, "failed to queue submit with error {}", meta::enum_to_string(vk_result));
    }

    bool vk_device_impl::vk_device_present(const device& _device, const sync& _sync, const swapchain& _swapchain, size_t _image_index) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_sync* sync_ptr = _sync.data.get<vk_sync>();
        vk_swapchain* swapchain_ptr = _swapchain.data.get<vk_swapchain>();

        uint32_t image_index = _image_index;

        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &sync_ptr->render_finished_semaphore[image_index],
            .swapchainCount = 1,
            .pSwapchains = &swapchain_ptr->swapchain,
            .pImageIndices = &image_index,
        };

        VkResult result = vkQueuePresentKHR(device_ptr->present_queue, &present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            return true;

        logger::assert_(result == VK_SUCCESS, detail::device_channel, "failed to queue present with error {}", meta::enum_to_string(result));

        return false;
    }

    void vk_device_impl::vk_device_wait(const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vkDeviceWaitIdle(device_ptr->device);
    }

    void vk_device_impl::vk_device_destroy(device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vkDestroyDevice(device_ptr->device, nullptr);
        detail::devices.remove(device_ptr);
        _device = {};
    }
} // namespace mars::graphics::vulkan