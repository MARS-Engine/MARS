#pragma once

#include <mars/graphics/backend/swapchain.hpp>

namespace mars::graphics::dx {
struct dx_swapchain_data;

struct dx_swapchain_impl {
	static swapchain dx_swapchain_create(const device& _device, const window& _window, const swapchain_create_params& _params);
	static void dx_swapchain_present(const swapchain& _swapchain, const device& _device);
	static size_t dx_swapchain_get_back_buffer_index(const swapchain& _swapchain);
	static void dx_swapchain_resize(swapchain& _swapchain, const device& _device, const vector2<size_t>& _size);
	static void dx_swapchain_destroy(swapchain& _swapchain, const device& _device);
};
} // namespace mars::graphics::dx
