#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/timeline_fence.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

namespace mars::graphics {

inline timeline_fence timeline_fence_create(const device& _device) {
	return _device.engine->get_impl<timeline_fence_impl>().timeline_fence_create(_device);
}

inline uint64_t timeline_fence_alloc_value(const timeline_fence& _fence) {
	return _fence.engine->get_impl<timeline_fence_impl>().timeline_fence_alloc_value(_fence);
}

inline void timeline_fence_signal(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value) {
	return _fence.engine->get_impl<timeline_fence_impl>().timeline_fence_signal_on_queue(_fence, _device, queue, value);
}

inline void timeline_fence_wait_gpu(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value) {
	return _fence.engine->get_impl<timeline_fence_impl>().timeline_fence_wait_on_queue(_fence, _device, queue, value);
}

inline void timeline_fence_wait_cpu(const timeline_fence& _fence, uint64_t value) {
	return _fence.engine->get_impl<timeline_fence_impl>().timeline_fence_cpu_wait(_fence, value);
}

inline uint64_t timeline_fence_completed(const timeline_fence& _fence) {
	return _fence.engine->get_impl<timeline_fence_impl>().timeline_fence_completed_value(_fence);
}

inline void timeline_fence_destroy(timeline_fence& _fence) {
	return _fence.engine->get_impl<timeline_fence_impl>().timeline_fence_destroy(_fence);
}

} // namespace mars::graphics
