#pragma once

#include <mars/graphics/functional/command_pool.hpp>

#include <type_traits>

namespace mars::graphics::object {

template <typename T>
inline void command_buffer_set_push_constants(
    const mars::command_buffer& cmd,
    const mars::pipeline& pipeline,
    const T& data) {
	static_assert(std::is_trivially_copyable_v<T>);
	static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Push-constant struct size must be a multiple of 4 bytes");
	mars::graphics::command_buffer_set_push_constants(
	    cmd,
	    pipeline,
	    reinterpret_cast<const uint32_t*>(&data),
	    sizeof(T) / sizeof(uint32_t));
}

template <typename T>
inline void command_buffer_set_compute_push_constants(
    const mars::command_buffer& cmd,
    const mars::compute_pipeline& pipeline,
    const T& data) {
	static_assert(std::is_trivially_copyable_v<T>);
	static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Push-constant struct size must be a multiple of 4 bytes");
	mars::graphics::command_buffer_set_compute_push_constants(
	    cmd,
	    pipeline,
	    reinterpret_cast<const uint32_t*>(&data),
	    sizeof(T) / sizeof(uint32_t));
}

} // namespace mars::graphics::object
