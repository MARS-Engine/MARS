#pragma once

#include <mars/graphics/backend/device.hpp>

namespace mars::graphics::vk {
struct vk_command_queue_impl {
	static void vk_command_queue_create(device& _device);
	static void vk_command_queue_destroy(device& _device);
};
} // namespace mars::graphics::vk
