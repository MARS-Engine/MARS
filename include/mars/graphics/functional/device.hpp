#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>
#include <mars/graphics/functional/window.hpp>

namespace mars::graphics {
inline device device_create(graphics_engine& _engine) {
	return _engine.get_impl<device_impl>().device_create(_engine);
}

inline void device_submit(const device& _device, const command_buffer& _command_buffer) {
	return _device.engine->get_impl<device_impl>().device_submit(_device, _command_buffer);
}

inline void device_flush(const device& _device) {
	return _device.engine->get_impl<device_impl>().device_flush(_device);
}

inline void device_destroy(device& _device) {
	return _device.engine->get_impl<device_impl>().device_destroy(_device);
}
} // namespace mars::graphics
