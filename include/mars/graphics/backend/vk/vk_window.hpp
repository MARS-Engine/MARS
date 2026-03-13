#pragma once

#include <mars/graphics/backend/window.hpp>

namespace mars::graphics::vk {
struct vk_window_impl {
	static window vk_window_create(graphics_engine& _engine, const window_params& _params);
	static void vk_window_destroy(window& _window);
};
} // namespace mars::graphics::vk
