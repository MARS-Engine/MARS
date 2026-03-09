#include "dx_internal.hpp"
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/dx12/dx_append_buffer.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars::graphics::dx {
static log_channel ab_channel("dx12");

append_buffer_base dx_append_buffer_impl::create(const device& dev, size_t elem_size, uint32_t capacity) {
	auto data = new dx_append_buffer_data();

	auto& buf_impl = dev.engine->get_impl<buffer_impl>();

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
							    .stride = 0,
							});

	data->zero_buf = buf_impl.buffer_create(dev, {
							 .buffer_type = MARS_BUFFER_TYPE_TRANSFER_SRC,
							 .buffer_property = MARS_BUFFER_PROPERTY_HOST_VISIBLE,
							 .allocated_size = sizeof(uint32_t),
							 .stride = 0,
						     });
	{
		auto* p = static_cast<uint32_t*>(buf_impl.buffer_map(data->zero_buf, dev, sizeof(uint32_t), 0));
		if (p) *p = 0u;
		buf_impl.buffer_unmap(data->zero_buf, dev);
	}

	append_buffer_base result;
	result.engine = dev.engine;
	result.data.store(data);
	return result;
}

void dx_append_buffer_impl::reset_counter(const append_buffer_base& ab, const command_buffer& cmd) {
	auto data = dx_expect_backend_data(ab.data.get<dx_append_buffer_data>(), __func__, "append_buffer.data");
	auto cb_data = dx_expect_backend_data(cmd.data.get<dx_command_buffer_data>(), __func__, "command_buffer.data");

	auto ctr = dx_expect_backend_data(data->counter_buf.data.get<dx_buffer_data>(), __func__, "counter_buffer.data");
	auto zero = dx_expect_backend_data(data->zero_buf.data.get<dx_buffer_data>(), __func__, "zero_buffer.data");

	if (ctr->dx12_state != D3D12_RESOURCE_STATE_COPY_DEST) {
		D3D12_RESOURCE_BARRIER to_copy = {};
		to_copy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		to_copy.Transition.pResource = ctr->resource.Get();
		to_copy.Transition.StateBefore = ctr->dx12_state;
		to_copy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		to_copy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cb_data->cmd_list->ResourceBarrier(1, &to_copy);
		ctr->dx12_state = D3D12_RESOURCE_STATE_COPY_DEST;
	}

	cb_data->cmd_list->CopyBufferRegion(ctr->resource.Get(), 0, zero->resource.Get(), 0, 4);

	{
		D3D12_RESOURCE_BARRIER to_uav = {};
		to_uav.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		to_uav.Transition.pResource = ctr->resource.Get();
		to_uav.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		to_uav.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		to_uav.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cb_data->cmd_list->ResourceBarrier(1, &to_uav);
		ctr->dx12_state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
}

const buffer& dx_append_buffer_impl::get_counter_buffer(const append_buffer_base& ab) {
	auto data = dx_expect_backend_data(ab.data.get<dx_append_buffer_data>(), __func__, "append_buffer.data");
	return data->counter_buf;
}

const buffer& dx_append_buffer_impl::get_data_buffer(const append_buffer_base& ab) {
	auto data = dx_expect_backend_data(ab.data.get<dx_append_buffer_data>(), __func__, "append_buffer.data");
	return data->data_buf;
}

void dx_append_buffer_impl::destroy(append_buffer_base& ab, const device& dev) {
	auto data = dx_expect_backend_data(ab.data.get<dx_append_buffer_data>(), __func__, "append_buffer.data");
	auto& buf_impl = dev.engine->get_impl<buffer_impl>();
	buf_impl.buffer_destroy(data->data_buf, dev);
	buf_impl.buffer_destroy(data->counter_buf, dev);
	buf_impl.buffer_destroy(data->zero_buf, dev);
	delete data;
	ab = {};
}
} // namespace mars::graphics::dx
