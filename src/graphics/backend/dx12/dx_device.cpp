#include <mars/graphics/backend/dx12/dx_device.hpp>

#include "dx_internal.hpp"

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/dx12/dx_command_queue.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel g_dx12_channel("dx12");

namespace {

void log_submit_device_removed_reason(ID3D12Device* _device, std::string_view _context) {
	if (_device == nullptr)
		return;

	const HRESULT removed_reason = _device->GetDeviceRemovedReason();
	if (removed_reason == S_OK)
		return;

	logger::error(g_dx12_channel, "DX12 device removed after {} (reason={:#x})", _context, static_cast<unsigned long>(removed_reason));
}

} // namespace

device dx_device_impl::dx_device_create(graphics_engine& _engine) {
	auto* device_data = new dx_device_data();

	Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
	const HRESULT debug_interface_hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
	if (SUCCEEDED(debug_interface_hr)) {
		debug_controller->EnableDebugLayer();
		logger::log(g_dx12_channel, "debug layer enabled");
	}
	else
		logger::warning(g_dx12_channel, "debug layer not available (hr={:#x})", static_cast<unsigned long>(debug_interface_hr));

	if (!dx_expect<CreateDXGIFactory2>(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&device_data->factory))) {
		delete device_data;
		return {};
	}

	if (!dx_expect<D3D12CreateDevice>(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device_data->device))) {
		delete device_data;
		return {};
	}

	if (SUCCEEDED(device_data->device.As(&device_data->device5))) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		if (SUCCEEDED(device_data->device5->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5))))
			device_data->supports_ray_tracing = options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
	heap_desc.NumDescriptors = dx_device_data::BINDLESS_TOTAL;
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (!dx_expect<&ID3D12Device::CreateDescriptorHeap>(device_data->device.Get(), &heap_desc, IID_PPV_ARGS(&device_data->bindless_heap))) {
		delete device_data;
		return {};
	}
	device_data->bindless_descriptor_size = device_data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	logger::log(g_dx12_channel, "global bindless heap created ({} slots)", dx_device_data::BINDLESS_TOTAL);

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;
	if (SUCCEEDED(device_data->device.As(&info_queue))) {
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, FALSE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE);
		logger::log(g_dx12_channel, "info queue configured (no break on error/corruption)");
	}

	device result = {};
	result.engine = _engine.allocator;
	result.data.store(device_data);

	dx_command_queue_impl::dx_command_queue_create(result);
	dx_command_queue_impl::dx_compute_queue_create(result);
	dx_command_queue_impl::dx_copy_queue_create(result);

	return result;
}

void dx_device_impl::dx_device_submit(const device& _device, const command_buffer& _command_buffer) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* queue_data = device_data->command_queue_data.expect<dx_command_queue_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	ID3D12CommandList* lists[] = {command_buffer_data->cmd_list.Get()};
	queue_data->cmd_queue->ExecuteCommandLists(1u, lists);
	log_submit_device_removed_reason(device_data->device.Get(), "direct queue ExecuteCommandLists");

	const UINT64 fence_value = ++queue_data->fence_value;
	queue_data->cmd_queue->Signal(queue_data->fence.Get(), fence_value);
	log_submit_device_removed_reason(device_data->device.Get(), "direct queue Signal");

	if (command_buffer_data->pool) {
		command_buffer_data->pool->submitted = true;
		command_buffer_data->pool->last_submitted_fence_value = fence_value;
	}
}

void dx_device_impl::dx_device_submit_compute(const device& _device, const command_buffer& _command_buffer) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* queue_data = device_data->compute_queue_data.expect<dx_command_queue_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	ID3D12CommandList* lists[] = {command_buffer_data->cmd_list.Get()};
	queue_data->cmd_queue->ExecuteCommandLists(1u, lists);
	log_submit_device_removed_reason(device_data->device.Get(), "compute queue ExecuteCommandLists");

	const UINT64 fence_value = ++queue_data->fence_value;
	queue_data->cmd_queue->Signal(queue_data->fence.Get(), fence_value);
	log_submit_device_removed_reason(device_data->device.Get(), "compute queue Signal");

	if (command_buffer_data->pool) {
		command_buffer_data->pool->submitted = true;
		command_buffer_data->pool->last_submitted_fence_value = fence_value;
	}
}

void dx_device_impl::dx_device_flush(const device& _device) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* queue_data = device_data->command_queue_data.expect<dx_command_queue_data>();

	const UINT64 value = ++queue_data->fence_value;
	queue_data->cmd_queue->Signal(queue_data->fence.Get(), value);

	if (queue_data->fence->GetCompletedValue() < value) {
		queue_data->fence->SetEventOnCompletion(value, queue_data->fence_event);
		WaitForSingleObject(queue_data->fence_event, INFINITE);
	}
}

bool dx_device_impl::dx_device_supports_feature(const device& _device, device_feature _feature) {
	auto* device_data = _device.data.get<dx_device_data>();
	if (!device_data)
		return false;

	switch (_feature) {
	case device_feature::ray_tracing:
		return device_data->supports_ray_tracing;
	default:
		return false;
	}
}

void dx_device_impl::dx_device_destroy(device& _device) {
	auto* device_data = _device.data.expect<dx_device_data>();
	if (device_data->copy_queue_data.get<void>() != nullptr)
		dx_command_queue_impl::dx_copy_queue_destroy(_device);
	if (device_data->compute_queue_data.get<void>() != nullptr)
		dx_command_queue_impl::dx_compute_queue_destroy(_device);
	if (device_data->command_queue_data.get<void>() != nullptr)
		dx_command_queue_impl::dx_command_queue_destroy(_device);
	delete device_data;
	_device = {};
}
} // namespace mars::graphics::dx
