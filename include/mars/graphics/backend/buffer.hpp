#pragma once

#include <mars/meta/type_erased.hpp>

#include <cstddef>

namespace mars {
struct device;
struct command_buffer;
struct graphics_backend_functions;

struct buffer {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
	size_t allocated_size = 0;
};

struct buffer_create_params {
	uint32_t buffer_type;
	uint32_t buffer_property;
	size_t allocated_size;
	size_t stride = 0;
};

enum mars_buffer_state {
	MARS_BUFFER_STATE_COMMON,
	MARS_BUFFER_STATE_SHADER_READ,
	MARS_BUFFER_STATE_UNORDERED_ACCESS,
	MARS_BUFFER_STATE_COPY_SOURCE,
	MARS_BUFFER_STATE_INDIRECT_ARGUMENT,
};

struct buffer_impl {
	buffer (*buffer_create)(const device& _device, const buffer_create_params& _params) = nullptr;
	void (*buffer_bind)(buffer& _buffer, const command_buffer& _command_buffer) = nullptr;
	void (*buffer_bind_index)(buffer& _buffer, const command_buffer& _command_buffer) = nullptr;
	void (*buffer_copy)(buffer& _buffer, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) = nullptr;
	void (*buffer_transition)(const command_buffer& _command_buffer, const buffer& _buffer, mars_buffer_state _state) = nullptr;
	void* (*buffer_map)(buffer& _buffer, const device& _device, size_t _size, size_t _offset) = nullptr;
	void (*buffer_unmap)(buffer& _buffer, const device& _device) = nullptr;
	void (*buffer_destroy)(buffer& _buffer, const device& _device) = nullptr;
	uint32_t (*buffer_get_uav_index)(const buffer& _buffer) = nullptr;
	uint32_t (*buffer_get_srv_index)(const buffer& _buffer) = nullptr;
	uint64_t (*buffer_get_device_address)(const buffer& _buffer) = nullptr;
};
} // namespace mars
