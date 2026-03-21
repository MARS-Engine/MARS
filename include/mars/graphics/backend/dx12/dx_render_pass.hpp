#pragma once

#include <mars/graphics/backend/render_pass.hpp>

namespace mars::graphics::dx {
struct dx_render_pass_impl {
	static render_pass dx_render_pass_create(const device& _device, const render_pass_create_params& _params);
	static void dx_render_pass_bind(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const depth_buffer* _depth_buffer, const render_pass_bind_param& _params);
	static void dx_render_pass_unbind(const render_pass& _render_pass, const command_buffer& _command_buffer);
	static void dx_render_pass_destroy(render_pass& _render_pass, const device& _device);
};
} // namespace mars::graphics::dx
