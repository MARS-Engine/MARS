#pragma once

#include <mars/graphics/backend/window.hpp>

#include <mars/graphics/functional/window.hpp>

namespace mars::graphics::dx {
struct dx_window_impl {
	static window dx_window_create(graphics_engine& _engine, const window_params& _params);
	static void dx_window_destroy(window& _window);
};
} // namespace mars::graphics::dx
