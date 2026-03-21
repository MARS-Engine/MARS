#pragma once

#include <mars/graphics/backend/format.hpp>
#include <mars/math/vector4.hpp>
#include <mars/meta/type_erased.hpp>

#include <cstddef>

enum mars_render_pass_load_op {
	MARS_RENDER_PASS_LOAD_OP_LOAD,
	MARS_RENDER_PASS_LOAD_OP_CLEAR
};

namespace mars {
struct device;
struct swapchain;
struct command_buffer;
struct framebuffer;
struct depth_buffer;
struct graphics_backend_functions;

struct render_pass {
	graphics_backend_functions* engine;
	meta::type_erased_ptr data;
};

struct render_pass_create_params {
	mars_format_type format;
	mars_depth_format depth_format = MARS_DEPTH_FORMAT_UNDEFINED;
	mars_render_pass_load_op load_operation = MARS_RENDER_PASS_LOAD_OP_CLEAR;
	float depth_clear_value = 1.0f;
};

struct render_pass_bind_param {
	size_t image_index;
	vector4<float> clear_color;
	float clear_depth = 1.0f;
};

struct render_pass_impl {
	render_pass (*render_pass_create)(const device& _device, const render_pass_create_params& _params) = nullptr;
	void (*render_pass_bind)(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const depth_buffer* _depth_buffer, const render_pass_bind_param& _params) = nullptr;
	void (*render_pass_unbind)(const render_pass& _render_pass, const command_buffer& _command_buffer) = nullptr;
	void (*render_pass_destroy)(render_pass& _render_pass, const device& _device) = nullptr;
};
} // namespace mars
