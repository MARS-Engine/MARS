#pragma once

#include <cstdint>
#include <mars/meta/type_erased.hpp>

namespace mars {
struct device;
struct buffer;
struct command_buffer;
struct graphics_backend_functions;

struct append_buffer_base {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct append_buffer_impl {
	append_buffer_base (*append_buffer_create)(const device& dev, size_t elem_size, uint32_t capacity) = nullptr;
	void (*append_buffer_reset_counter)(const append_buffer_base& ab, const command_buffer& cmd) = nullptr;
	const buffer& (*append_buffer_get_counter_buffer)(const append_buffer_base& ab) = nullptr;
	const buffer& (*append_buffer_get_data_buffer)(const append_buffer_base& ab) = nullptr;
	void (*append_buffer_destroy)(append_buffer_base& ab, const device& dev) = nullptr;
};

} // namespace mars
