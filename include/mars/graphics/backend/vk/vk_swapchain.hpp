#pragma once

#include <mars/graphics/backend/swapchain.hpp>

namespace mars::graphics::vk {
struct vk_swapchain_impl {
	static swapchain vk_swapchain_create(const device& _device, const window& _window, const swapchain_create_params& _params);
	static void vk_swapchain_present(const swapchain& _swapchain, const device& _device);
	static size_t vk_swapchain_get_back_buffer_index(const swapchain& _swapchain);
	static void vk_swapchain_resize(swapchain& _swapchain, const device& _device, const vector2<size_t>& _size);
	static void vk_swapchain_destroy(swapchain& _swapchain, const device& _device);
};
} // namespace mars::graphics::vk
