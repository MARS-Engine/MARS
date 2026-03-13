#pragma once

#include <mars/graphics/backend/indirect_executor.hpp>

namespace mars::graphics::vk {
struct vk_indirect_executor_impl {
	static indirect_executor create(const device& dev, mars_command_signature_type type);
	static indirect_executor create_with_constant(const device& dev, const pipeline& pipe);
	static indirect_executor create_with_constant_compute(const device& dev, const compute_pipeline& pipe);
	static void record_dispatch(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf);
	static void record_draw_indexed(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf);
	static void destroy(indirect_executor& ex);
};
} // namespace mars::graphics::vk
