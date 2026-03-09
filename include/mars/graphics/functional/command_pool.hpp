#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
namespace graphics {

inline command_pool command_pool_create(const device& _device) {
	return _device.engine->get_impl<command_pool_impl>().command_pool_create(_device);
}

inline std::vector<command_buffer> command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _size) {
	return _device.engine->get_impl<command_pool_impl>().command_buffer_create(_command_pool, _device, _size);
}

inline void command_buffer_record(const command_buffer& _command_buffer) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_record(_command_buffer);
}

inline void command_buffer_reset(const command_buffer& _command_buffer) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_reset(_command_buffer);
}

inline void command_buffer_record_end(const command_buffer& _command_buffer) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_record_end(_command_buffer);
}

inline void command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_draw(_command_buffer, _params);
}

inline void command_buffer_draw_indexed(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_draw_indexed(_command_buffer, _params);
}

inline void command_buffer_dispatch(const command_buffer& _command_buffer, const command_buffer_dispatch_params& _params) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_dispatch(_command_buffer, _params);
}

inline void command_buffer_set_push_constants(const command_buffer& _command_buffer, const pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_set_push_constants(_command_buffer, _pipeline, _values, _count);
}

inline void command_buffer_set_compute_push_constants(const command_buffer& _command_buffer, const compute_pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_set_compute_push_constants(_command_buffer, _pipeline, _values, _count);
}

inline void command_buffer_memory_barrier(const command_buffer& _command_buffer) {
	return _command_buffer.engine->get_impl<command_pool_impl>().command_buffer_memory_barrier(_command_buffer);
}

inline void command_pool_destroy(command_pool& _command_pool, const device& _device) {
	return _command_pool.engine->get_impl<command_pool_impl>().command_pool_destroy(_command_pool, _device);
}

inline void command_buffer_begin_event(const command_buffer& _cmd, std::string_view _name) {
	return _cmd.engine->get_impl<command_pool_impl>().command_buffer_begin_event(_cmd, _name);
}

inline void command_buffer_end_event(const command_buffer& _cmd) {
	return _cmd.engine->get_impl<command_pool_impl>().command_buffer_end_event(_cmd);
}
} // namespace graphics
} // namespace mars
