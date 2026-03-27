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

inline bool device_supports_feature(const device& _device, device_feature _feature) {
	return _device.engine->get_impl<device_impl>().device_supports_feature(_device, _feature);
}

inline void device_destroy(device& _device) {
	return _device.engine->get_impl<device_impl>().device_destroy(_device);
}

inline void device_submit_compute(const device& _device, const command_buffer& _command_buffer) {
	return _device.engine->get_impl<device_impl>().device_submit_compute(_device, _command_buffer);
}
} // namespace mars::graphics
