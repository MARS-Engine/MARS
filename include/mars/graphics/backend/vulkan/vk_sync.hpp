#pragma once

#include <mars/graphics/backend/sync.hpp>
#include <mars/graphics/functional/device.hpp>
#include <mars/graphics/functional/window.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_sync {
        std::vector<VkSemaphore> image_available_semaphore;
        std::vector<VkSemaphore> render_finished_semaphore;
        std::vector<VkFence> in_flight_fence;
    };

    struct vk_sync_impl {
        static sync vk_sync_create(const device& _device, size_t _size);
        static void vk_sync_wait(const sync& _sync, const device& _device, size_t _current_frame);
        static void vk_sync_reset(const sync& _sync, const device& _device, size_t _current_frame);
        static bool vk_sync_get_next_image(const sync& _sync, const device& _device, const swapchain& _swapchain, size_t _current_frame, size_t& _image_index);
        static void vk_sync_destroy(sync& _sync, const device& _device);
    };
} // namespace mars::graphics::vulkan