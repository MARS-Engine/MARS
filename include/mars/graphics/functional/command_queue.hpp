#pragma once

#include <mars/graphics/backend/command_queue.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

namespace mars::graphics {
inline void command_queue_create(device& _device) {
	return _device.engine->get_impl<command_queue_impl>().command_queue_create(_device);
}

inline void command_queue_destroy(device& _device) {
	return _device.engine->get_impl<command_queue_impl>().command_queue_destroy(_device);
}
} // namespace mars::graphics
