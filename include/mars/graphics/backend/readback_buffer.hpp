#pragma once

#include <mars/meta/type_erased.hpp>

#include <cstddef>
#include <cstdint>

namespace mars {
struct device;
struct buffer;
struct graphics_backend_functions;

struct readback_ticket {
	uint32_t slot_index = 0;
	uint64_t fence_value = 0;
	size_t data_size = 0;
};

struct readback_buffer {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct readback_buffer_impl {
	readback_buffer (*readback_buffer_create)(const device& dev, size_t slot_size, uint32_t num_slots) = nullptr;
	readback_ticket (*readback_buffer_schedule)(readback_buffer& rb, const buffer& src, size_t src_offset, size_t size) = nullptr;
	uint64_t (*readback_buffer_flush)(readback_buffer& rb) = nullptr;
	bool (*readback_buffer_try_read)(const readback_buffer& rb, const readback_ticket& ticket, const void** out_ptr) = nullptr;
	void (*readback_buffer_release_slot)(readback_buffer& rb, const readback_ticket& ticket) = nullptr;
	void (*readback_buffer_destroy)(readback_buffer& rb) = nullptr;
};
} // namespace mars
