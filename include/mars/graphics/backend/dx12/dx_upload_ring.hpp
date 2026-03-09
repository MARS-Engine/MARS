#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/upload_ring.hpp>

namespace mars::graphics::dx {
struct dx_upload_ring_impl {
	static mars::upload_ring create(const mars::device& _device, size_t capacity);
	static void schedule(mars::upload_ring& _ring, const mars::buffer& dst, size_t dst_offset, const void* src, size_t size);
	static uint64_t flush(mars::upload_ring& _ring);
	static void cpu_wait(const mars::upload_ring& _ring, uint64_t fence_value);
	static void destroy(mars::upload_ring& _ring);
};
} // namespace mars::graphics::dx
