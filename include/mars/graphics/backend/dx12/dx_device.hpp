#pragma once

#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
struct dx_device_data;

struct dx_device_impl {
	static device dx_device_create(graphics_engine& _engine);
	static void dx_device_submit(const device& _device, const command_buffer& _command_buffer);
	static void dx_device_flush(const device& _device);
	static bool dx_device_supports_feature(const device& _device, device_feature feature);
	static void dx_device_destroy(device& _device);
};
} // namespace mars::graphics::dx
