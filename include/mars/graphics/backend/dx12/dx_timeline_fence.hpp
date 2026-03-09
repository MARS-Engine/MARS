#pragma once

#include <mars/graphics/backend/timeline_fence.hpp>

namespace mars::graphics::dx {
struct dx_timeline_fence_impl {
	static timeline_fence create(const device& _device);
	static uint64_t alloc_value(const timeline_fence& _fence);
	static void signal_on_queue(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value);
	static void wait_on_queue(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value);
	static void cpu_wait(const timeline_fence& _fence, uint64_t value);
	static uint64_t completed_value(const timeline_fence& _fence);
	static void destroy(timeline_fence& _fence);
};
} // namespace mars::graphics::dx
