#pragma once

#include <mars/graphics/backend/buffer.hpp>

namespace mars::graphics::vk {
struct vk_buffer_impl {
	static buffer vk_buffer_create(const device& _device, const buffer_create_params& _params);
	static void vk_buffer_bind(buffer& _buffer, const command_buffer& _command_buffer);
	static void vk_buffer_bind_index(buffer& _buffer, const command_buffer& _command_buffer);
	static void vk_buffer_copy(buffer& _buffer, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset);
	static void vk_buffer_transition(const command_buffer& _command_buffer, const buffer& _buffer, mars_buffer_state _state);
	static void* vk_buffer_map(buffer& _buffer, const device& _device, size_t _size, size_t _offset);
	static void vk_buffer_unmap(buffer& _buffer, const device& _device);
	static void vk_buffer_destroy(buffer& _buffer, const device& _device);
	static uint32_t vk_buffer_get_uav_index(const buffer& _buffer);
	static uint32_t vk_buffer_get_srv_index(const buffer& _buffer);
};
} // namespace mars::graphics::vk
