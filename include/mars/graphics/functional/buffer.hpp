#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
namespace graphics {

inline buffer buffer_create(const device& _device, const buffer_create_params& _params) {
	return _device.engine->get_impl<buffer_impl>().buffer_create(_device, _params);
}

inline void buffer_bind(buffer& _buffer, const command_buffer& _command_buffer) {
	_buffer.engine->get_impl<buffer_impl>().buffer_bind(_buffer, _command_buffer);
}

inline void buffer_bind_index(buffer& _buffer, const command_buffer& _command_buffer) {
	_buffer.engine->get_impl<buffer_impl>().buffer_bind_index(_buffer, _command_buffer);
}

inline void buffer_copy(buffer& _buffer, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) {
	_buffer.engine->get_impl<buffer_impl>().buffer_copy(_buffer, _src_buffer, _command_buffer, _offset);
}

inline void* buffer_map(buffer& _buffer, const device& _device, size_t _size, size_t _offset) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_map(_buffer, _device, _size, _offset);
}

inline uint32_t buffer_get_uav_index(const buffer& _buffer) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_get_uav_index(_buffer);
}

inline uint32_t buffer_get_srv_index(const buffer& _buffer) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_get_srv_index(_buffer);
}

inline void buffer_transition(const command_buffer& _command_buffer, const buffer& _buffer, mars_buffer_state _state) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_transition(_command_buffer, _buffer, _state);
}

inline void buffer_unmap(buffer& _buffer, const device& _device) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_unmap(_buffer, _device);
}

inline void buffer_destroy(buffer& _buffer, const device& _device) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_destroy(_buffer, _device);
}

inline uint64_t buffer_get_device_address(const buffer& _buffer) {
	return _buffer.engine->get_impl<buffer_impl>().buffer_get_device_address(_buffer);
}

} // namespace graphics
} // namespace mars
