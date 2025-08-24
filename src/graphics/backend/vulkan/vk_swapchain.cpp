#include <mars/graphics/backend/vulkan/vk_swapchain.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {

    namespace detail {
        sparse_vector<vk_swapchain, 5> swapchains;
        log_channel swapchain_channel("graphics/vulkan/swapchain");
    } // namespace detail

    swapchain vk_swapchain_impl::vk_swapchain_create(instance& _instance, window& _window) {
        vk_swapchain* swapchain_ptr = detail::swapchains.request_entry();

        swapchain result;
        result.data = swapchain_ptr;
        result.engine = _instance.engine;
        return result;
    }

    void vk_swapchain_impl::vk_swapchain_destroy(swapchain& _swapchain, instance& _instance) {
        vk_swapchain* swapchain_ptr = static_cast<vk_swapchain*>(_swapchain.data);
        vk_instance* instance_ptr = static_cast<vk_instance*>(_instance.data);
        detail::swapchains.remove(swapchain_ptr);
    }
} // namespace mars::graphics::vulkan