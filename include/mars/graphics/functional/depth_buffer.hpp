#pragma once

#include <mars/graphics/backend/depth_buffer.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

namespace mars::graphics {

inline depth_buffer depth_buffer_create(const device& _device, const depth_buffer_create_params& _params) {
	return _device.engine->get_impl<depth_buffer_impl>().depth_buffer_create(_device, _params);
}

inline void depth_buffer_transition(const command_buffer& _command_buffer, depth_buffer& _depth_buffer, mars_texture_state _before, mars_texture_state _after) {
	return _depth_buffer.engine->get_impl<depth_buffer_impl>().depth_buffer_transition(_command_buffer, _depth_buffer, _before, _after);
}

inline uint32_t depth_buffer_get_srv_index(const depth_buffer& _depth_buffer) {
	return _depth_buffer.engine->get_impl<depth_buffer_impl>().depth_buffer_get_srv_index(_depth_buffer);
}

inline void depth_buffer_destroy(depth_buffer& _depth_buffer, const device& _device) {
	return _depth_buffer.engine->get_impl<depth_buffer_impl>().depth_buffer_destroy(_depth_buffer, _device);
}

} // namespace mars::graphics
