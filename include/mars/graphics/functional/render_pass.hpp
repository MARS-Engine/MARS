#pragma once

#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/render_pass.hpp>

namespace mars {
namespace graphics {

inline render_pass render_pass_create(const device& _device, const render_pass_create_params& _params) {
	return _device.engine->get_impl<render_pass_impl>().render_pass_create(_device, _params);
}

inline void render_pass_bind(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const render_pass_bind_param& _params) {
	_render_pass.engine->get_impl<render_pass_impl>().render_pass_bind(_render_pass, _command_buffer, _framebuffer, _params);
}
inline void render_pass_unbind(const render_pass& _render_pass, const command_buffer& _command_buffer) {
	_render_pass.engine->get_impl<render_pass_impl>().render_pass_unbind(_render_pass, _command_buffer);
}

inline void render_pass_destroy(render_pass& _render_pass, const device& _device) {
	_render_pass.engine->get_impl<render_pass_impl>().render_pass_destroy(_render_pass, _device);
}
} // namespace graphics
} // namespace mars
