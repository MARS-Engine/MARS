#pragma once

#include <mars/graphics/backend/depth_buffer.hpp>

namespace mars::graphics::dx {
struct dx_depth_buffer_impl {
	static depth_buffer dx_depth_buffer_create(const device& _device, const depth_buffer_create_params& _params);
	static void dx_depth_buffer_transition(const command_buffer& _command_buffer, depth_buffer& _depth_buffer, mars_texture_state _before, mars_texture_state _after);
	static uint32_t dx_depth_buffer_get_srv_index(const depth_buffer& _depth_buffer);
	static void dx_depth_buffer_destroy(depth_buffer& _depth_buffer, const device& _device);
};
} // namespace mars::graphics::dx
