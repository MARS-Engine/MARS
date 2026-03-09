#pragma once

#include <mars/graphics/backend/framebuffer.hpp>

namespace mars::graphics::dx {
struct dx_framebuffer_impl {
	static framebuffer dx_framebuffer_create(const device& _device, const framebuffer_create_params& _params);
	static std::vector<framebuffer> dx_framebuffer_create_from_swapchain(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass);
	static void dx_framebuffer_destroy(framebuffer& _framebuffer, const device& _device);
};
} // namespace mars::graphics::dx
