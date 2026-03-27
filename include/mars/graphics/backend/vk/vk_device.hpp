#pragma once

#include <mars/graphics/backend/device.hpp>

namespace mars::graphics::vk {
struct vk_device_impl {
	static device vk_device_create(graphics_engine& _engine);
	static void vk_device_submit(const device& _device, const command_buffer& _command_buffer);
	static void vk_device_flush(const device& _device);
	static bool vk_device_supports_feature(const device& _device, device_feature _feature);
	static void vk_device_destroy(device& _device);
	static void vk_device_submit_compute(const device& _device, const command_buffer& _command_buffer);
};
} // namespace mars::graphics::vk
