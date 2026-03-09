#pragma once

#include <mars/graphics/backend/texture.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

namespace mars::graphics {
inline texture texture_create(const device& _device, const texture_create_params& _params) {
	return _device.engine->get_impl<texture_impl>().texture_create(_device, _params);
}

inline void texture_copy(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) {
	return _texture.engine->get_impl<texture_impl>().texture_copy(_texture, _src_buffer, _command_buffer, _offset);
}

inline void texture_transition(const command_buffer& _command_buffer, texture& _texture, mars_texture_state _before, mars_texture_state _after) {
	return _texture.engine->get_impl<texture_impl>().texture_transition(_command_buffer, _texture, _before, _after);
}

inline void* texture_map(texture& _texture, const device& _device) {
	return _texture.engine->get_impl<texture_impl>().texture_map(_texture, _device);
}

inline void texture_unmap(texture& _texture, const device& _device) {
	return _texture.engine->get_impl<texture_impl>().texture_unmap(_texture, _device);
}

inline texture_upload_layout texture_get_upload_layout(texture& _texture, const device& _device) {
	return _texture.engine->get_impl<texture_impl>().texture_get_upload_layout(_texture, _device);
}

inline void texture_destroy(texture& _texture, const device& _device) {
	return _texture.engine->get_impl<texture_impl>().texture_destroy(_texture, _device);
}
} // namespace mars::graphics
