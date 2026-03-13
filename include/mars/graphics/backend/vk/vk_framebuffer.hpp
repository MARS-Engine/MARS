#pragma once

#include <mars/graphics/backend/framebuffer.hpp>

namespace mars::graphics::vk {
struct vk_framebuffer_impl {
	static framebuffer vk_framebuffer_create(const device& _device, const framebuffer_create_params& _params);
	static std::vector<framebuffer> vk_framebuffer_create_from_swapchain(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass);
	static void vk_framebuffer_destroy(framebuffer& _frame_buffer, const device& _device);
};
} // namespace mars::graphics::vk
