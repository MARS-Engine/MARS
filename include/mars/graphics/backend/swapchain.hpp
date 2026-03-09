#pragma once

#include "format.hpp"
#include "texture.hpp"
#include <mars/math/vector2.hpp>

namespace mars {
struct graphics_backend_functions;
struct window;
struct device;

struct swapchain_create_params {
	size_t buffer_count = 2;
};

struct swapchain {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
	size_t swapchain_size = 0;
	mars_format_type format;
	vector2<size_t> extent;
	texture_view view;
};

struct swapchain_impl {
	swapchain (*swapchain_create)(const device& _device, const window& _window, const swapchain_create_params& _params) = nullptr;
	void (*swapchain_present)(const swapchain& _swapchain, const device& _device) = nullptr;
	size_t (*swapchain_get_back_buffer_index)(const swapchain& _swapchain) = nullptr;
	void (*swapchain_resize)(swapchain& _swapchain, const device& _device, const vector2<size_t>& _size) = nullptr;
	void (*swapchain_destroy)(swapchain& _swapchain, const device& _device) = nullptr;
};
} // namespace mars
