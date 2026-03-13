#pragma once

#include <mars/graphics/backend/readback_buffer.hpp>

namespace mars::graphics::vk {
struct vk_readback_buffer_impl {
	static readback_buffer create(const device& dev, size_t slot_size, uint32_t num_slots);
	static readback_ticket schedule(readback_buffer& rb, const buffer& src, size_t src_offset, size_t size);
	static uint64_t flush(readback_buffer& rb);
	static bool try_read(const readback_buffer& rb, const readback_ticket& ticket, const void** out_ptr);
	static void release_slot(readback_buffer& rb, const readback_ticket& ticket);
	static void destroy(readback_buffer& rb);
};
} // namespace mars::graphics::vk
