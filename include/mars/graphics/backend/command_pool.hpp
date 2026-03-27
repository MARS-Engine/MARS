#pragma once

#include <mars/meta/type_erased.hpp>
#include <mars/graphics/backend/ray_tracing_types.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace mars {
struct device;
struct graphics_backend_functions;
struct texture;
struct pipeline;
struct compute_pipeline;
struct ray_tracing_pipeline;

struct command_pool {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct command_buffer {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
	size_t buffer_index = 0;
};

struct command_buffer_draw_params {
	size_t vertex_count;
	size_t instance_count;
	size_t first_vertex;
	size_t first_instance;
};

struct command_buffer_draw_indexed_params {
	size_t index_count;
	size_t instance_count;
	size_t first_index;
	size_t vertex_offset;
	size_t first_instance;
};

struct command_buffer_dispatch_params {
	size_t group_count_x;
	size_t group_count_y;
	size_t group_count_z;
};

struct command_pool_impl {
	command_pool (*command_pool_create)(const device& _device) = nullptr;
	std::vector<command_buffer> (*command_buffer_create)(const command_pool& _command_pool, const device& _device, size_t _n_command_buffers) = nullptr;
	void (*command_buffer_reset)(const command_buffer& _command_buffer) = nullptr;
	void (*command_buffer_record)(const command_buffer& _command_buffer) = nullptr;
	void (*command_buffer_record_end)(const command_buffer& _command_buffer) = nullptr;
	void (*command_buffer_draw)(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) = nullptr;
	void (*command_buffer_draw_indexed)(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params) = nullptr;
	void (*command_buffer_dispatch)(const command_buffer& _command_buffer, const command_buffer_dispatch_params& _params) = nullptr;
	void (*command_buffer_set_push_constants)(const command_buffer& _command_buffer, const pipeline& _pipeline, const uint32_t* _values, size_t _count) = nullptr;
	void (*command_buffer_set_compute_push_constants)(const command_buffer& _command_buffer, const compute_pipeline& _pipeline, const uint32_t* _values, size_t _count) = nullptr;
	void (*command_buffer_memory_barrier)(const command_buffer& _command_buffer) = nullptr;
	void (*command_pool_destroy)(command_pool& _command_pool, const device& _device) = nullptr;

	void (*command_buffer_begin_event)(const command_buffer& _command_buffer, std::string_view _name) = nullptr;
	void (*command_buffer_end_event)(const command_buffer& _command_buffer) = nullptr;

	void (*command_buffer_trace_rays)(const command_buffer& _command_buffer, const ray_tracing_pipeline& _pipeline, const rt_dispatch_regions& _regions, uint32_t _width, uint32_t _height, uint32_t _depth) = nullptr;

	command_pool (*compute_command_pool_create)(const device& _device) = nullptr;
};
} // namespace mars
