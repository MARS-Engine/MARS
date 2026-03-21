#pragma once

#include <mars/graphics/backend/depth_buffer.hpp>

namespace mars::graphics::vk {
struct vk_depth_buffer_impl {
	static depth_buffer vk_depth_buffer_create(const device& _device, const depth_buffer_create_params& _params);
	static void vk_depth_buffer_transition(const command_buffer& _command_buffer, depth_buffer& _depth_buffer, mars_texture_state _before, mars_texture_state _after);
	static uint32_t vk_depth_buffer_get_srv_index(const depth_buffer& _depth_buffer);
	static void vk_depth_buffer_destroy(depth_buffer& _depth_buffer, const device& _device);
};
} // namespace mars::graphics::vk
