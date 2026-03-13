#pragma once

#include <mars/graphics/backend/upload_ring.hpp>

namespace mars::graphics::vk {
struct vk_upload_ring_impl {
	static upload_ring create(const device& _device, size_t capacity);
	static void schedule(upload_ring& _ring, const buffer& dst, size_t dst_offset, const void* src, size_t size);
	static uint64_t flush(upload_ring& _ring);
	static void cpu_wait(const upload_ring& _ring, uint64_t fence_value);
	static void destroy(upload_ring& _ring);
};
} // namespace mars::graphics::vk
