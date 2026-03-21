#pragma once

#include <mars/graphics/backend/format.hpp>
#include <mars/graphics/backend/texture.hpp>
#include <mars/math/vector2.hpp>
#include <mars/meta/type_erased.hpp>

namespace mars {
struct graphics_backend_functions;
struct device;
struct command_buffer;

struct depth_buffer {
	graphics_backend_functions* engine = nullptr;
	meta::type_erased_ptr data;
	vector2<size_t> size = {};
	mars_depth_format format = MARS_DEPTH_FORMAT_UNDEFINED;
};

struct depth_buffer_create_params {
	vector2<size_t> size = {};
	mars_depth_format format = MARS_DEPTH_FORMAT_D32_SFLOAT;
	float clear_depth = 1.0f;
	bool sampled = true;
};

struct depth_buffer_impl {
	depth_buffer (*depth_buffer_create)(const device& _device, const depth_buffer_create_params& _params) = nullptr;
	void (*depth_buffer_transition)(const command_buffer& _command_buffer, depth_buffer& _depth_buffer, mars_texture_state _before, mars_texture_state _after) = nullptr;
	uint32_t (*depth_buffer_get_srv_index)(const depth_buffer& _depth_buffer) = nullptr;
	void (*depth_buffer_destroy)(depth_buffer& _depth_buffer, const device& _device) = nullptr;
};
} // namespace mars
