#include <mars/graphics/backend/dx12/dx_command_pool.hpp>

#include "dx_internal.hpp"

#include <mars/graphics/backend/ray_tracing_pipeline.hpp>
#include <mars/graphics/functional/device.hpp>

#include <mars/debug/logger.hpp>

#include <WinPixEventRuntime/pix3.h>

#include <string>

namespace mars::graphics::dx {
static log_channel g_dx12_channel("dx12");

namespace {
constexpr DWORD k_fence_wait_timeout_ms = 5000u;

void log_device_removed_reason(dx_command_queue_data* _queue_data) {
	if (_queue_data == nullptr || _queue_data->cmd_queue == nullptr)
		return;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	if (FAILED(_queue_data->cmd_queue->GetDevice(IID_PPV_ARGS(&device))) || !device)
		return;

	const HRESULT removed_reason = device->GetDeviceRemovedReason();
	if (removed_reason != S_OK)
		logger::error(g_dx12_channel, "DX12 device removed while waiting on a fence (reason={:#x})", static_cast<unsigned long>(removed_reason));
}
} // namespace

command_pool dx_command_pool_impl::dx_command_pool_create(const device& _device) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* pool_data = new dx_command_pool_data();
	pool_data->queue = device_data->command_queue_data.expect<dx_command_queue_data>();

	dx_expect<&ID3D12Device::CreateCommandAllocator>(device_data->device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pool_data->cmd_allocator));

	command_pool result = {};
	result.engine = _device.engine;
	result.data.store(pool_data);
	return result;
}

std::vector<command_buffer> dx_command_pool_impl::dx_command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _n) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* pool_data = _command_pool.data.expect<dx_command_pool_data>();
	std::vector<command_buffer> result;
	result.reserve(_n);

	for (size_t index = 0u; index < _n; ++index) {
		auto* command_buffer_data = new dx_command_buffer_data();
		command_buffer_data->pool = pool_data;
		command_buffer_data->device_data = device_data;
		command_buffer_data->bindless_heap_raw = device_data->bindless_heap.Get();

		if (!dx_expect<&ID3D12Device::CreateCommandList>(device_data->device.Get(), 0u, pool_data->cmd_list_type, pool_data->cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&command_buffer_data->cmd_list))) {
			delete command_buffer_data;
			continue;
		}

		command_buffer_data->cmd_list->QueryInterface(IID_PPV_ARGS(&command_buffer_data->cmd_list4));
		dx_expect<&ID3D12GraphicsCommandList::Close>(command_buffer_data->cmd_list.Get());

		command_buffer command_buffer = {};
		command_buffer.engine = _device.engine;
		command_buffer.data.store(command_buffer_data);
		command_buffer.buffer_index = index;
		result.push_back(command_buffer);
	}

	return result;
}

command_pool dx_command_pool_impl::dx_compute_command_pool_create(const device& _device) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* pool_data = new dx_command_pool_data();
	pool_data->queue = device_data->compute_queue_data.expect<dx_command_queue_data>();
	pool_data->cmd_list_type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

	if (!dx_expect<&ID3D12Device::CreateCommandAllocator>(device_data->device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&pool_data->cmd_allocator)))
		return {};

	command_pool result = {};
	result.engine = _device.engine;
	result.data.store(pool_data);
	return result;
}

void dx_command_pool_impl::dx_command_buffer_reset(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!command_buffer_data->pool || !command_buffer_data->pool->cmd_allocator || !command_buffer_data->cmd_list) {
		logger::error(g_dx12_channel, "command_buffer_reset called with invalid command buffer state");
		return;
	}

	if (command_buffer_data->pool->submitted) {
		if (command_buffer_data->pool->queue && command_buffer_data->pool->queue->fence && command_buffer_data->pool->queue->fence->GetCompletedValue() < command_buffer_data->pool->last_submitted_fence_value) {
			const HRESULT event_hr = command_buffer_data->pool->queue->fence->SetEventOnCompletion(command_buffer_data->pool->last_submitted_fence_value, command_buffer_data->pool->queue->fence_event);
			if (FAILED(event_hr)) {
				logger::error(
					g_dx12_channel,
					"SetEventOnCompletion failed while waiting for fence value {} (hr={:#x})",
					command_buffer_data->pool->last_submitted_fence_value,
					static_cast<unsigned long>(event_hr)
				);
				log_device_removed_reason(command_buffer_data->pool->queue);
				return;
			}

			const DWORD wait_result = WaitForSingleObject(command_buffer_data->pool->queue->fence_event, k_fence_wait_timeout_ms);
			if (wait_result != WAIT_OBJECT_0) {
				if (wait_result == WAIT_TIMEOUT)
					logger::error(g_dx12_channel, "Timed out waiting {} ms for DX12 fence value {}", static_cast<unsigned long>(k_fence_wait_timeout_ms), command_buffer_data->pool->last_submitted_fence_value);
				else
					logger::error(g_dx12_channel, "WaitForSingleObject failed while waiting for fence value {} (result={}, last_error={})", command_buffer_data->pool->last_submitted_fence_value, static_cast<unsigned long>(wait_result), static_cast<unsigned long>(GetLastError()));
				log_device_removed_reason(command_buffer_data->pool->queue);
				return;
			}
		}

		if (!dx_expect<&ID3D12CommandAllocator::Reset>(command_buffer_data->pool->cmd_allocator.Get()))
			return;
		command_buffer_data->pool->submitted = false;
	}

	dx_expect<&ID3D12GraphicsCommandList::Reset>(command_buffer_data->cmd_list.Get(), command_buffer_data->pool->cmd_allocator.Get(), nullptr);
}

void dx_command_pool_impl::dx_command_buffer_record(const command_buffer&) {
}

void dx_command_pool_impl::dx_command_buffer_record_end(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	command_buffer_data->cmd_list->Close();
}

void dx_command_pool_impl::dx_command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	command_buffer_data->cmd_list->DrawInstanced(static_cast<UINT>(_params.vertex_count), static_cast<UINT>(_params.instance_count), static_cast<UINT>(_params.first_vertex), static_cast<UINT>(_params.first_instance));
}

void dx_command_pool_impl::dx_command_buffer_draw_indexed(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	command_buffer_data->cmd_list->DrawIndexedInstanced(static_cast<UINT>(_params.index_count), static_cast<UINT>(_params.instance_count), static_cast<UINT>(_params.first_index), static_cast<INT>(_params.vertex_offset), static_cast<UINT>(_params.first_instance));
}

void dx_command_pool_impl::dx_command_buffer_dispatch(const command_buffer& _command_buffer, const command_buffer_dispatch_params& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	command_buffer_data->cmd_list->Dispatch(static_cast<UINT>(_params.group_count_x), static_cast<UINT>(_params.group_count_y), static_cast<UINT>(_params.group_count_z));
}

void dx_command_pool_impl::dx_command_buffer_set_push_constants(const command_buffer& _command_buffer, const pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto* pipeline_data = _pipeline.data.expect<dx_pipeline_data>();
	if (!pipeline_data->has_push_constants)
		return;

	const UINT count = static_cast<UINT>(_count < pipeline_data->push_constant_count ? _count : pipeline_data->push_constant_count);
	command_buffer_data->cmd_list->SetGraphicsRoot32BitConstants(static_cast<UINT>(pipeline_data->push_constants_root_index), count, _values, 0u);
}

void dx_command_pool_impl::dx_command_buffer_set_compute_push_constants(const command_buffer& _command_buffer, const compute_pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto* pipeline_data = _pipeline.data.expect<dx_compute_pipeline_data>();
	if (!pipeline_data->has_push_constants)
		return;

	const UINT count = static_cast<UINT>(_count < pipeline_data->push_constant_count ? _count : pipeline_data->push_constant_count);
	command_buffer_data->cmd_list->SetComputeRoot32BitConstants(static_cast<UINT>(pipeline_data->push_constants_root_index), count, _values, 0u);
}

void dx_command_pool_impl::dx_command_buffer_memory_barrier(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = nullptr;
	command_buffer_data->cmd_list->ResourceBarrier(1u, &barrier);
}

void dx_command_pool_impl::dx_command_pool_destroy(command_pool& _command_pool, const device&) {
	auto* pool_data = _command_pool.data.expect<dx_command_pool_data>();
	delete pool_data;
	_command_pool = {};
}

void dx_command_pool_impl::dx_command_buffer_begin_event(const command_buffer& _command_buffer, std::string_view _name) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!command_buffer_data->cmd_list)
		return;
	PIXBeginEvent(command_buffer_data->cmd_list.Get(), 0ull, std::string(_name).c_str());
}

void dx_command_pool_impl::dx_command_buffer_end_event(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!command_buffer_data->cmd_list)
		return;
	PIXEndEvent(command_buffer_data->cmd_list.Get());
}

void dx_command_pool_impl::dx_command_buffer_trace_rays(const command_buffer& _command_buffer, const mars::ray_tracing_pipeline&, const mars::rt_dispatch_regions& _regions, uint32_t _width, uint32_t _height, uint32_t _depth) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	auto to_record = [](const mars::rt_dispatch_region& _region) -> D3D12_GPU_VIRTUAL_ADDRESS_RANGE {
		return {_region.address, _region.size};
	};
	auto to_range_stride = [](const mars::rt_dispatch_region& _region) -> D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE {
		return {_region.address, _region.size, _region.stride};
	};

	D3D12_DISPATCH_RAYS_DESC desc = {};
	desc.RayGenerationShaderRecord = to_record(_regions.raygen);
	desc.MissShaderTable = to_range_stride(_regions.miss);
	desc.HitGroupTable = to_range_stride(_regions.hit);
	desc.CallableShaderTable = to_range_stride(_regions.callable);
	desc.Width = _width;
	desc.Height = _height;
	desc.Depth = _depth;
	command_buffer_data->cmd_list4->DispatchRays(&desc);
}
} // namespace mars::graphics::dx
