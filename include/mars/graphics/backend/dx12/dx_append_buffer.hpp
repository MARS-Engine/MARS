#pragma once

#include <mars/graphics/backend/append_buffer.hpp>

namespace mars::graphics::dx {
struct dx_append_buffer_impl {
	static append_buffer_base create(const device& dev, size_t elem_size, uint32_t capacity);
	static void reset_counter(const append_buffer_base& ab, const command_buffer& cmd);
	static const buffer& get_counter_buffer(const append_buffer_base& ab);
	static const buffer& get_data_buffer(const append_buffer_base& ab);
	static void destroy(append_buffer_base& ab, const device& dev);
};
} // namespace mars::graphics::dx
