#include "mars/graphics/backend/vulkan/vk_swapchain.hpp"
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_sync.hpp>

#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/meta.hpp>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_sync, 2> syncs;
        log_channel sync_channel("graphics/vulkan/sync");
    } // namespace detail

    sync vk_sync_impl::vk_sync_create(const device& _device, size_t _size) {
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_sync* sync_ptr = detail::syncs.request_entry();

        sync result;
        result.engine = _device.engine;
        result.data = sync_ptr;
        VkSemaphoreCreateInfo semaphore_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fence_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        sync_ptr->image_available_semaphore.resize(_size);
        sync_ptr->render_finished_semaphore.resize(_size);
        sync_ptr->in_flight_fence.resize(_size);

        for (size_t i = 0; i < _size; i++) {
            VkResult vk_result = vkCreateSemaphore(device_ptr->device, &semaphore_info, nullptr, &sync_ptr->image_available_semaphore[i]);

            logger::assert_(vk_result == VK_SUCCESS, detail::sync_channel, "Failed to create Semaphore with error {}", meta::enum_to_string(vk_result));

            vk_result = vkCreateSemaphore(device_ptr->device, &semaphore_info, nullptr, &sync_ptr->render_finished_semaphore[i]);

            logger::assert_(vk_result == VK_SUCCESS, detail::sync_channel, "Failed to create Semaphore with error {}", meta::enum_to_string(vk_result));

            vk_result = vkCreateFence(device_ptr->device, &fence_info, nullptr, &sync_ptr->in_flight_fence[i]);

            logger::assert_(vk_result == VK_SUCCESS, detail::sync_channel, "Failed to create Fence with error {}", meta::enum_to_string(vk_result));
        }
        return result;
    }

    void vk_sync_impl::vk_sync_wait(const sync& _sync, const device& _device, size_t _current_frame) {
        vk_sync* sync_ptr = static_cast<vk_sync*>(_sync.data);
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);

        vkWaitForFences(device_ptr->device, 1, &sync_ptr->in_flight_fence[_current_frame], VK_TRUE, UINT64_MAX);
    }

    void vk_sync_impl::vk_sync_reset(const sync& _sync, const device& _device, size_t _current_frame) {
        vk_sync* sync_ptr = static_cast<vk_sync*>(_sync.data);
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);

        vkResetFences(device_ptr->device, 1, &sync_ptr->in_flight_fence[_current_frame]);
    }

    bool vk_sync_impl::vk_sync_get_next_image(const sync& _sync, const device& _device, const swapchain& _swapchain, size_t _current_frame, size_t& _image_index) {
        vk_sync* sync_ptr = static_cast<vk_sync*>(_sync.data);
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_swapchain* swapchain_ptr = static_cast<vk_swapchain*>(_swapchain.data);

        uint32_t image_index;

        VkResult result = vkAcquireNextImageKHR(device_ptr->device, swapchain_ptr->swapchain, UINT64_MAX, sync_ptr->image_available_semaphore[_current_frame], VK_NULL_HANDLE, &image_index);

        _image_index = image_index;

        // needs resize
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
            return true;

        logger::assert_(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, detail::sync_channel, "failed to present swap chain image with error {}", meta::enum_to_string(result));

        return false;
    }

    void vk_sync_impl::vk_sync_destroy(sync& _sync, const device& _device) {
        vk_sync* sync_ptr = static_cast<vk_sync*>(_sync.data);
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);

        for (size_t i = 0; i < sync_ptr->image_available_semaphore.size(); i++) {
            vkDestroySemaphore(device_ptr->device, sync_ptr->image_available_semaphore[i], nullptr);
            vkDestroySemaphore(device_ptr->device, sync_ptr->render_finished_semaphore[i], nullptr);
            vkDestroyFence(device_ptr->device, sync_ptr->in_flight_fence[i], nullptr);
        }
        detail::syncs.remove(sync_ptr);
        _sync = {};
    }
} // namespace mars::graphics::vulkan