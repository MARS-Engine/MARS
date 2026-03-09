#pragma once

#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/compute_pipeline.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/indirect_executor.hpp>
#include <mars/graphics/backend/pipeline.hpp>

#include <cstdint>

namespace mars::graphics {
inline indirect_executor indirect_executor_create(const device& dev, mars_command_signature_type type) {
	return dev.engine->get_impl<indirect_executor_impl>().indirect_executor_create(dev, type);
}

inline indirect_executor indirect_executor_create_with_constant(const device& dev, const pipeline& pipe) {
	return dev.engine->get_impl<indirect_executor_impl>().indirect_executor_create_with_constant(dev, pipe);
}

inline indirect_executor indirect_executor_create_with_constant(const device& dev, const compute_pipeline& pipe) {
	return dev.engine->get_impl<indirect_executor_impl>().indirect_executor_create_with_constant_compute(dev, pipe);
}

inline void indirect_executor_record_dispatch(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf = nullptr) {
	return ex.engine->get_impl<indirect_executor_impl>().indirect_executor_record_dispatch(ex, cmd, arg_buf, max_count, count_buf);
}

inline void indirect_executor_record_draw_indexed(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf = nullptr) {
	return ex.engine->get_impl<indirect_executor_impl>().indirect_executor_record_draw_indexed(ex, cmd, arg_buf, max_count, count_buf);
}

inline void indirect_executor_destroy(indirect_executor& ex) {
	return ex.engine->get_impl<indirect_executor_impl>().indirect_executor_destroy(ex);
}
} // namespace mars::graphics
