#include "dx_internal.hpp"
#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/dx12/dx_command_queue.hpp>
#include <mars/graphics/backend/dx12/dx_device.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

device dx_device_impl::dx_device_create(graphics_engine& _engine) {
	auto data = new dx_device_data();

	Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
	if (SUCCEEDED(hr)) {
		debug_controller->EnableDebugLayer();
		logger::log(dx12_channel, "debug layer enabled");
	} 
	else
		logger::warning(dx12_channel, "debug layer not available (hr={:#x})", (unsigned long)hr);

	hr = dx_expect<CreateDXGIFactory2>(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&data->factory));

	hr = dx_expect<D3D12CreateDevice>(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&data->device));

	{
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.NumDescriptors = dx_device_data::BINDLESS_TOTAL;
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		hr = dx_expect<&ID3D12Device::CreateDescriptorHeap>(data->device.Get(), &heap_desc, IID_PPV_ARGS(&data->bindless_heap));
		data->bindless_descriptor_size = data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		logger::log(dx12_channel, "global bindless heap created ({} slots)", dx_device_data::BINDLESS_TOTAL);
	}

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;
	if (SUCCEEDED(data->device.As(&info_queue))) {
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, FALSE);
		info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, FALSE);
		logger::log(dx12_channel, "info queue configured (no break on error/corruption)");
	}

	device result;
	result.engine = _engine.allocator;
	result.data.store(data);

	dx_command_queue_impl::dx_command_queue_create(result);
	dx_command_queue_impl::dx_compute_queue_create(result);
	dx_command_queue_impl::dx_copy_queue_create(result);

	return result;
}

static void print_debug_messages(ID3D12Device* device) {
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&info_queue)))) {
		UINT64 count = info_queue->GetNumStoredMessages();
		for (UINT64 i = 0; i < count; i++) {
			SIZE_T msg_size = 0;
			info_queue->GetMessage(i, nullptr, &msg_size);
			if (msg_size > 0) {
				auto msg = (D3D12_MESSAGE*)malloc(msg_size);
				info_queue->GetMessage(i, msg, &msg_size);
				switch (msg->Severity) {
				case D3D12_MESSAGE_SEVERITY_ERROR:
				case D3D12_MESSAGE_SEVERITY_CORRUPTION:
					logger::error(dx12_channel, "{}", msg->pDescription);
					break;
				case D3D12_MESSAGE_SEVERITY_WARNING:
					logger::warning(dx12_channel, "{}", msg->pDescription);
					break;
				default:
					logger::log(dx12_channel, "{}", msg->pDescription);
					break;
				}
				free(msg);
			}
		}
		info_queue->ClearStoredMessages();
	}
}

void dx_device_impl::dx_device_submit(const device& _device, const command_buffer& _command_buffer) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto cq_data = device_data->command_queue_data.expect<dx_command_queue_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();

	ID3D12CommandList* lists[] = {cb_data->cmd_list.Get()};
	cq_data->cmd_queue->ExecuteCommandLists(1, lists);

	const UINT64 fence_value = ++cq_data->fence_value;
	cq_data->cmd_queue->Signal(cq_data->fence.Get(), fence_value);

	if (cb_data->pool) {
		cb_data->pool->submitted = true;
		cb_data->pool->last_submitted_fence_value = fence_value;
	}
}

void dx_device_impl::dx_device_flush(const device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto cq_data = device_data->command_queue_data.expect<dx_command_queue_data>();

	const UINT64 val = ++cq_data->fence_value;
	cq_data->cmd_queue->Signal(cq_data->fence.Get(), val);

	if (cq_data->fence->GetCompletedValue() < val) {
		cq_data->fence->SetEventOnCompletion(val, cq_data->fence_event);
		WaitForSingleObject(cq_data->fence_event, INFINITE);
	}

	print_debug_messages(device_data->device.Get());
}

bool dx_device_impl::dx_device_supports_feature(const device&, device_feature) {
	return false;
}

void dx_device_impl::dx_device_destroy(device& _device) {
	auto data = _device.data.expect<dx_device_data>();
	if (data->copy_queue_data.get<void>() != nullptr)
		dx_command_queue_impl::dx_copy_queue_destroy(_device);
	if (data->compute_queue_data.get<void>() != nullptr)
		dx_command_queue_impl::dx_compute_queue_destroy(_device);
	if (data->command_queue_data.get<void>() != nullptr)
		dx_command_queue_impl::dx_command_queue_destroy(_device);
	delete data;
	_device = {};
}
} // namespace mars::graphics::dx
