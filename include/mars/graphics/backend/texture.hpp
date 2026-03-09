#pragma once

#include <cstddef>
#include <mars/graphics/backend/format.hpp>
#include <mars/math/vector2.hpp>
#include <mars/math/vector4.hpp>
#include <mars/meta/type_erased.hpp>
#include <mars/meta/type_erased_fn.hpp>

namespace mars {
struct graphics_backend_functions;
struct device;
struct buffer;
struct command_buffer;

enum mars_texture_state {
	MARS_TEXTURE_STATE_COPY_DST,
	MARS_TEXTURE_STATE_SHADER_READ,
	MARS_TEXTURE_STATE_UNORDERED_ACCESS,
	MARS_TEXTURE_STATE_COMMON,
};

struct texture_view {
	meta::type_erased_ptr data;
	meta::type_erased_fn view_extractor;
};

struct texture {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
	texture_view view;
	vector2<size_t> size;
	unsigned char channels;
	unsigned char format_size;
};

struct texture_create_params {
	vector2<size_t> size;
	mars_format_type format;
	mars_texture_usage usage = MARS_TEXTURE_USAGE_SAMPLED | MARS_TEXTURE_USAGE_TRANSFER_DST;
	mars_texture_filter_mode filter = MARS_TEXTURE_FILTER_LINEAR;
	mars_texture_type texture_type = MARS_TEXTURE_TYPE_2D;
	size_t mip_levels = 1;

	size_t array_size = 1;
	vector4<float> clear_color = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct texture_upload_layout {
	size_t row_count;
	size_t row_size;
	size_t row_pitch;
	size_t offset;
};

struct texture_impl {
	texture (*texture_create)(const device& _device, const texture_create_params& _params) = nullptr;
	void (*texture_copy)(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset);
	void (*texture_transition)(const command_buffer& _command_buffer, texture& _texture, mars_texture_state _before, mars_texture_state _after) = nullptr;
	void* (*texture_map)(texture& _texture, const device& _device) = nullptr;
	void (*texture_unmap)(texture& _texture, const device& _device) = nullptr;
	texture_upload_layout (*texture_get_upload_layout)(texture& _texture, const device& _device) = nullptr;
	uint32_t (*texture_get_srv_index)(const texture& _texture) = nullptr;
	uint32_t (*texture_get_uav_base)(const texture& _texture) = nullptr;
	void (*texture_destroy)(texture& _texture, const device& _device) = nullptr;
};
} // namespace mars
