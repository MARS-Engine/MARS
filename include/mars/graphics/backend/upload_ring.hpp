#pragma once

#include <cstddef>
#include <cstdint>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/meta/type_erased.hpp>

namespace mars {
struct device;
struct graphics_backend_functions;

struct upload_ring {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct upload_ring_impl {
	upload_ring (*upload_ring_create)(const device& _device, size_t capacity) = nullptr;
	void (*upload_ring_schedule)(upload_ring& _ring, const buffer& dst, size_t dst_offset, const void* src, size_t size) = nullptr;
	uint64_t (*upload_ring_flush)(upload_ring& _ring) = nullptr;
	void (*upload_ring_cpu_wait)(const upload_ring& _ring, uint64_t fence_value) = nullptr;
	void (*upload_ring_destroy)(upload_ring& _ring) = nullptr;
};
} // namespace mars
