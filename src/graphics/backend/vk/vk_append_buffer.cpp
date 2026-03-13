#include "vk_internal.hpp"

#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/vk/vk_append_buffer.hpp>
#include <mars/graphics/backend/vk/vk_buffer.hpp>

namespace mars::graphics::vk {
append_buffer_base vk_append_buffer_impl::create(const device& dev, size_t elem_size, uint32_t capacity) {
	auto* data = new vk_append_buffer_data();
	mars::buffer_impl& buf_impl = dev.engine->get_impl<mars::buffer_impl>();

	data->data_buf = buf_impl.buffer_create(dev, {
													 .buffer_type = MARS_BUFFER_TYPE_STRUCTURED | MARS_BUFFER_TYPE_UNORDERED_ACCESS,
													 .buffer_property = MARS_BUFFER_PROPERTY_DEVICE_LOCAL,
													 .allocated_size = elem_size * capacity,
													 .stride = elem_size,
												 });
	data->counter_buf = buf_impl.buffer_create(dev, {
														.buffer_type = MARS_BUFFER_TYPE_UNORDERED_ACCESS,
														.buffer_property = MARS_BUFFER_PROPERTY_DEVICE_LOCAL,
														.allocated_size = sizeof(uint32_t),
														.stride = 0u,
													});
	data->zero_buf = buf_impl.buffer_create(dev, {
													 .buffer_type = MARS_BUFFER_TYPE_TRANSFER_SRC,
													 .buffer_property = MARS_BUFFER_PROPERTY_HOST_VISIBLE,
													 .allocated_size = sizeof(uint32_t),
													 .stride = 0u,
												 });

	if (auto* zero_ptr = static_cast<uint32_t*>(buf_impl.buffer_map(data->zero_buf, dev, sizeof(uint32_t), 0u))) {
		*zero_ptr = 0u;
		buf_impl.buffer_unmap(data->zero_buf, dev);
	}

	append_buffer_base result;
	result.engine = dev.engine;
	result.data.store(data);
	return result;
}

void vk_append_buffer_impl::reset_counter(const append_buffer_base& ab, const command_buffer& cmd) {
	auto* data = ab.data.expect<vk_append_buffer_data>();
	auto* command_buffer_data = cmd.data.expect<vk_command_buffer_data>();
	auto* counter_data = data->counter_buf.data.expect<vk_buffer_data>();
	auto* zero_data = data->zero_buf.data.expect<vk_buffer_data>();

	vk_buffer_impl::vk_buffer_transition(cmd, data->zero_buf, MARS_BUFFER_STATE_COPY_SOURCE);

	vk_cmd_pipeline_buffer_barrier2(
		command_buffer_data->command_buffer,
		counter_data->current_stage,
		counter_data->current_access,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		counter_data->buffer,
		0u,
		counter_data->size
	);

	VkBufferCopy copy = {};
	copy.size = sizeof(uint32_t);
	vkCmdCopyBuffer(command_buffer_data->command_buffer, zero_data->buffer, counter_data->buffer, 1u, &copy);

	vk_cmd_pipeline_buffer_barrier2(
		command_buffer_data->command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		counter_data->buffer,
		0u,
		counter_data->size
	);
	counter_data->current_access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	counter_data->current_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
}

const buffer& vk_append_buffer_impl::get_counter_buffer(const append_buffer_base& ab) {
	auto* data = ab.data.expect<vk_append_buffer_data>();
	return data->counter_buf;
}

const buffer& vk_append_buffer_impl::get_data_buffer(const append_buffer_base& ab) {
	auto* data = ab.data.expect<vk_append_buffer_data>();
	return data->data_buf;
}

void vk_append_buffer_impl::destroy(append_buffer_base& ab, const device& dev) {
	auto* data = ab.data.expect<vk_append_buffer_data>();
	mars::buffer_impl& buf_impl = dev.engine->get_impl<mars::buffer_impl>();
	buf_impl.buffer_destroy(data->data_buf, dev);
	buf_impl.buffer_destroy(data->counter_buf, dev);
	buf_impl.buffer_destroy(data->zero_buf, dev);
	delete data;
	ab = {};
}
} // namespace mars::graphics::vk
