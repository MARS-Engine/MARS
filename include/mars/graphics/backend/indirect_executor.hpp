#pragma once

#include <mars/meta/type_erased.hpp>

#include <cstdint>

enum mars_command_signature_type {
	MARS_COMMAND_SIGNATURE_DISPATCH,
	MARS_COMMAND_SIGNATURE_DRAW_INDEXED,
};

namespace mars {
struct device;
struct buffer;
struct command_buffer;
struct pipeline;
struct compute_pipeline;
struct graphics_backend_functions;

struct indirect_executor {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
};

struct indirect_executor_impl {
	indirect_executor (*indirect_executor_create)(const device& dev, mars_command_signature_type type) = nullptr;

	indirect_executor (*indirect_executor_create_with_constant)(const device& dev, const pipeline& pipe) = nullptr;

	indirect_executor (*indirect_executor_create_with_constant_compute)(const device& dev, const compute_pipeline& pipe) = nullptr;
	void (*indirect_executor_record_dispatch)(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) = nullptr;
	void (*indirect_executor_record_draw_indexed)(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) = nullptr;
	void (*indirect_executor_destroy)(indirect_executor& ex) = nullptr;
};
} // namespace mars
