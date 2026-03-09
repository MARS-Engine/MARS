#pragma once

#include <cstdint>
#include <mars/graphics/backend/command_queue.hpp>
#include <mars/meta/type_erased.hpp>

namespace mars {
struct graphics_backend_functions;
struct device;

struct timeline_fence {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct timeline_fence_impl {
	timeline_fence (*timeline_fence_create)(const device& _device) = nullptr;
	uint64_t (*timeline_fence_alloc_value)(const timeline_fence& _fence) = nullptr;
	void (*timeline_fence_signal_on_queue)(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value) = nullptr;
	void (*timeline_fence_wait_on_queue)(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value) = nullptr;
	void (*timeline_fence_cpu_wait)(const timeline_fence& _fence, uint64_t value) = nullptr;
	uint64_t (*timeline_fence_completed_value)(const timeline_fence& _fence) = nullptr;
	void (*timeline_fence_destroy)(timeline_fence& _fence) = nullptr;
};
} // namespace mars
