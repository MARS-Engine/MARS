#pragma once

#include <mars/graphics/backend/texture.hpp>

namespace mars::graphics::vk {
struct vk_texture_impl {
	static texture vk_texture_create(const device& _device, const texture_create_params& _params);
	static void vk_texture_copy(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset);
	static void vk_texture_transition(const command_buffer& _command_buffer, texture& _texture, mars_texture_state _before, mars_texture_state _after);
	static void* vk_texture_map(texture& _texture, const device& _device);
	static void vk_texture_unmap(texture& _texture, const device& _device);
	static texture_upload_layout vk_texture_get_upload_layout(texture& _texture, const device& _device);
	static uint32_t vk_texture_get_srv_index(const texture& _texture);
	static uint32_t vk_texture_get_uav_base(const texture& _texture);
	static void vk_texture_destroy(texture& _texture, const device& _device);
};
} // namespace mars::graphics::vk
