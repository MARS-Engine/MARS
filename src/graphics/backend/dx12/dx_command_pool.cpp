#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_command_pool.hpp>
#include <mars/graphics/functional/device.hpp>

#include <mars/debug/logger.hpp>

#include <WinPixEventRuntime/pix3.h>

#include <algorithm>
#include <string>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

command_pool dx_command_pool_impl::dx_command_pool_create(const device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto data = new dx_command_pool_data();

	dx_expect<&ID3D12Device::CreateCommandAllocator>(device_data->device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&data->cmd_allocator));

	command_pool result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

std::vector<command_buffer> dx_command_pool_impl::dx_command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _n) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto pool_data = _command_pool.data.expect<dx_command_pool_data>();
	std::vector<command_buffer> result;
	result.reserve(_n);

	for (size_t i = 0; i < _n; i++) {
		auto data = new dx_command_buffer_data();
		data->pool = pool_data;
		data->bindless_heap_raw = device_data->bindless_heap.Get();

		HRESULT hr = dx_expect<&ID3D12Device::CreateCommandList>(
			device_data->device.Get(), 0, D3D12_COMMAND_LIST_TYPE_DIRECT, pool_data->cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&data->cmd_list)
		);

		if (data->cmd_list) {
			dx_expect<&ID3D12GraphicsCommandList::Close>(data->cmd_list.Get());
		}

		command_buffer cb;
		cb.engine = _device.engine;
		cb.data.store(data);
		cb.buffer_index = i;
		result.push_back(cb);
	}

	return result;
}

void dx_command_pool_impl::dx_command_buffer_reset(const command_buffer& _command_buffer) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!data->pool || !data->pool->cmd_allocator || !data->cmd_list) {
		logger::error(dx12_channel, "command_buffer_reset called with invalid command buffer state");
		return;
	}

	if (data->pool->submitted) {
		HRESULT hr = dx_expect<&ID3D12CommandAllocator::Reset>(data->pool->cmd_allocator.Get());
		if (FAILED(hr))
			return;
		data->pool->submitted = false;
	}

	dx_expect<&ID3D12GraphicsCommandList::Reset>(data->cmd_list.Get(), data->pool->cmd_allocator.Get(), nullptr);
}

void dx_command_pool_impl::dx_command_buffer_record(const command_buffer& _command_buffer) {
}

void dx_command_pool_impl::dx_command_buffer_record_end(const command_buffer& _command_buffer) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	data->cmd_list->Close();
}

void dx_command_pool_impl::dx_command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	data->cmd_list->DrawInstanced((UINT)_params.vertex_count, (UINT)_params.instance_count, (UINT)_params.first_vertex, (UINT)_params.first_instance);
}

void dx_command_pool_impl::dx_command_buffer_draw_indexed(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	data->cmd_list->DrawIndexedInstanced((UINT)_params.index_count, (UINT)_params.instance_count, (UINT)_params.first_index, (INT)_params.vertex_offset, (UINT)_params.first_instance);
}

void dx_command_pool_impl::dx_command_buffer_dispatch(const command_buffer& _command_buffer, const command_buffer_dispatch_params& _params) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	data->cmd_list->Dispatch((UINT)_params.group_count_x, (UINT)_params.group_count_y, (UINT)_params.group_count_z);
}

void dx_command_pool_impl::dx_command_buffer_set_push_constants(const command_buffer& _command_buffer, const pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto pipe_data = _pipeline.data.expect<dx_pipeline_data>();
	if (!pipe_data->has_push_constants)
		return;

	const auto count = (UINT)(_count < pipe_data->push_constant_count ? _count : pipe_data->push_constant_count);
	data->cmd_list->SetGraphicsRoot32BitConstants((UINT)pipe_data->push_constants_root_index, count, _values, 0);
}

void dx_command_pool_impl::dx_command_buffer_set_compute_push_constants(const command_buffer& _command_buffer, const compute_pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto pipe_data = _pipeline.data.expect<dx_compute_pipeline_data>();
	if (!pipe_data->has_push_constants)
		return;

	const auto count = (UINT)(_count < pipe_data->push_constant_count ? _count : pipe_data->push_constant_count);
	data->cmd_list->SetComputeRoot32BitConstants((UINT)pipe_data->push_constants_root_index, count, _values, 0);
}

void dx_command_pool_impl::dx_command_buffer_memory_barrier(const command_buffer& _command_buffer) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = nullptr;
	data->cmd_list->ResourceBarrier(1, &barrier);
}

void dx_command_pool_impl::dx_command_pool_destroy(command_pool& _command_pool, const device& _device) {
	auto data = _command_pool.data.expect<dx_command_pool_data>();
	delete data;
	_command_pool = {};
}

void dx_command_pool_impl::dx_command_buffer_begin_event(const command_buffer& _command_buffer, std::string_view _name) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!data->cmd_list) return;
	PIXBeginEvent(data->cmd_list.Get(), 0ull, std::string(_name).c_str());
}

void dx_command_pool_impl::dx_command_buffer_end_event(const command_buffer& _command_buffer) {
	auto data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!data->cmd_list) return;
	PIXEndEvent(data->cmd_list.Get());
}
} // namespace mars::graphics::dx
