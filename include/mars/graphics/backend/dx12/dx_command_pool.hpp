#pragma once

#include <mars/graphics/backend/command_pool.hpp>

namespace mars::graphics::dx {
struct dx_command_pool_impl {
	static command_pool dx_command_pool_create(const device& _device);
	static std::vector<command_buffer> dx_command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _n);
	static void dx_command_buffer_reset(const command_buffer& _command_buffer);
	static void dx_command_buffer_record(const command_buffer& _command_buffer);
	static void dx_command_buffer_record_end(const command_buffer& _command_buffer);
	static void dx_command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params);
	static void dx_command_buffer_draw_indexed(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params);
	static void dx_command_buffer_dispatch(const command_buffer& _command_buffer, const command_buffer_dispatch_params& _params);
	static void dx_command_buffer_set_push_constants(const command_buffer& _command_buffer, const pipeline& _pipeline, const uint32_t* _values, size_t _count);
	static void dx_command_buffer_set_compute_push_constants(const command_buffer& _command_buffer, const compute_pipeline& _pipeline, const uint32_t* _values, size_t _count);
	static void dx_command_buffer_memory_barrier(const command_buffer& _command_buffer);
	static void dx_command_pool_destroy(command_pool& _command_pool, const device& _device);
	static void dx_command_buffer_begin_event(const command_buffer& _command_buffer, std::string_view _name);
	static void dx_command_buffer_end_event(const command_buffer& _command_buffer);
};
} // namespace mars::graphics::dx
