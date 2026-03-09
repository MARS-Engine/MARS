#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/framebuffer.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/swapchain.hpp>

namespace mars {
namespace graphics {

inline framebuffer framebuffer_create(const device& _device, const framebuffer_create_params _params) {
	return _device.engine->get_impl<framebuffer_impl>().framebuffer_create(_device, _params);
}

inline std::vector<mars::framebuffer> framebuffer_create_from_swapchain(const mars::device& device, const mars::swapchain& swapchain, const mars::render_pass& render_pass) {
	return device.engine->get_impl<framebuffer_impl>().framebuffer_create_from_swapchain(device, swapchain, render_pass);
}

inline void framebuffer_destroy(framebuffer& _framebuffer, const device& _device) {
	_framebuffer.engine->get_impl<framebuffer_impl>().framebuffer_destroy(_framebuffer, _device);
}
} // namespace graphics
} // namespace mars
