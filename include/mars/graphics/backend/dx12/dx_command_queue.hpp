#pragma once

#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
struct dx_command_queue_data;

struct dx_command_queue_impl {
	static void dx_command_queue_create(device& _device);
	static void dx_command_queue_destroy(device& _device);

	static void dx_compute_queue_create(device& _device);
	static void dx_copy_queue_create(device& _device);
	static void dx_compute_queue_destroy(device& _device);
	static void dx_copy_queue_destroy(device& _device);
};
} // namespace mars::graphics::dx
