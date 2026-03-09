#pragma once

#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/backend/window.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

namespace mars::graphics {
inline swapchain swapchain_create(const device& _device, const window& _window, const swapchain_create_params& _params = {}) {
	return _device.engine->get_impl<swapchain_impl>().swapchain_create(_device, _window, _params);
}

inline void swapchain_present(const swapchain& _swapchain, const device& _device) {
	return _swapchain.engine->get_impl<swapchain_impl>().swapchain_present(_swapchain, _device);
}

inline size_t swapchain_get_back_buffer_index(const swapchain& _swapchain) {
	return _swapchain.engine->get_impl<swapchain_impl>().swapchain_get_back_buffer_index(_swapchain);
}

inline void swapchain_resize(swapchain& _swapchain, const device& _device, const vector2<size_t>& _size) {
	return _swapchain.engine->get_impl<swapchain_impl>().swapchain_resize(_swapchain, _device, _size);
}

inline void swapchain_destroy(swapchain& _swapchain, const device& _device) {
	return _swapchain.engine->get_impl<swapchain_impl>().swapchain_destroy(_swapchain, _device);
}
} // namespace mars::graphics
