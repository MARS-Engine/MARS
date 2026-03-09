#pragma once

#include <mars/graphics/backend/render_pass.hpp>
#include <mars/graphics/backend/texture.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>

#include <cstddef>
#include <vector>

namespace mars {
struct device;
struct swapchain;
struct graphics_backend_functions;

struct framebuffer {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
	vector2<size_t> extent;
};

struct framebuffer_create_params {
	texture_view view;
	render_pass render_pass;
	vector2<size_t> size;
};

struct framebuffer_impl {
	framebuffer (*framebuffer_create)(const device& _device, const framebuffer_create_params& _params) = nullptr;
	std::vector<framebuffer> (*framebuffer_create_from_swapchain)(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass) = nullptr;
	void (*framebuffer_destroy)(framebuffer& _frame_buffer, const device& _device) = nullptr;
};
} // namespace mars
