#include "dx_bindless_allocator.hpp"
#include "dx_internal.hpp"

#include <mars/graphics/backend/dx12/dx_depth_buffer.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
namespace {
D3D12_RESOURCE_STATES depth_state_to_dx12(mars_texture_state state) {
	switch (state) {
	case MARS_TEXTURE_STATE_SHADER_READ:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case MARS_TEXTURE_STATE_COMMON:
		return D3D12_RESOURCE_STATE_COMMON;
	default:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
}
} // namespace

depth_buffer dx_depth_buffer_impl::dx_depth_buffer_create(const device& _device, const depth_buffer_create_params& _params) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* data = new dx_depth_buffer_data();
	data->resource_format = dx_format_from_mars(dx_resource_format_from_depth(_params.format));
	data->depth_format = _params.format;
	data->sampled = _params.sampled;
	data->dx12_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	D3D12_RESOURCE_DESC depth_desc = {};
	depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_desc.Width = static_cast<UINT64>(_params.size.x);
	depth_desc.Height = static_cast<UINT>(_params.size.y);
	depth_desc.DepthOrArraySize = 1;
	depth_desc.MipLevels = 1;
	depth_desc.Format = data->resource_format;
	depth_desc.SampleDesc.Count = 1;
	depth_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heap_props = {};
	heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clear_value = {};
	clear_value.Format = dx_depth_format_from_mars(_params.format);
	clear_value.DepthStencil.Depth = _params.clear_depth;

	const HRESULT create_resource_hr = dx_expect<&ID3D12Device::CreateCommittedResource>(
		device_data->device.Get(),
		&heap_props,
		D3D12_HEAP_FLAG_NONE,
		&depth_desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clear_value,
		IID_PPV_ARGS(&data->resource)
	);
	if (FAILED(create_resource_hr)) {
		delete data;
		return {};
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
	dsv_heap_desc.NumDescriptors = 1;
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	const HRESULT create_dsv_heap_hr = dx_expect<&ID3D12Device::CreateDescriptorHeap>(
		device_data->device.Get(),
		&dsv_heap_desc,
		IID_PPV_ARGS(&data->dsv_heap)
	);
	if (FAILED(create_dsv_heap_hr)) {
		delete data;
		return {};
	}
	data->dsv_handle = data->dsv_heap->GetCPUDescriptorHandleForHeapStart();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
	dsv_desc.Format = dx_depth_format_from_mars(_params.format);
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device_data->device->CreateDepthStencilView(data->resource.Get(), &dsv_desc, data->dsv_handle);

	if (_params.sampled) {
		const UINT srv_slot = dx_allocate_bindless_srv_slot(device_data);
		data->srv_bindless_idx = srv_slot;
		D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu = device_data->bindless_heap->GetCPUDescriptorHandleForHeapStart();
		srv_cpu.ptr += static_cast<SIZE_T>(srv_slot) * device_data->bindless_descriptor_size;

		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = dx_srv_format_from_depth(_params.format);
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = 1;
		device_data->device->CreateShaderResourceView(data->resource.Get(), &srv_desc, srv_cpu);
	}

	depth_buffer result;
	result.engine = _device.engine;
	result.data.store(data);
	result.size = _params.size;
	result.format = _params.format;
	return result;
}

void dx_depth_buffer_impl::dx_depth_buffer_transition(const command_buffer& _command_buffer, depth_buffer& _depth_buffer, mars_texture_state, mars_texture_state _after) {
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto* depth_data = _depth_buffer.data.expect<dx_depth_buffer_data>();
	const D3D12_RESOURCE_STATES after_state = depth_state_to_dx12(_after);
	if (depth_data->dx12_state == after_state)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = depth_data->resource.Get();
	barrier.Transition.StateBefore = depth_data->dx12_state;
	barrier.Transition.StateAfter = after_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_buffer_data->cmd_list->ResourceBarrier(1, &barrier);
	depth_data->dx12_state = after_state;
}

uint32_t dx_depth_buffer_impl::dx_depth_buffer_get_srv_index(const depth_buffer& _depth_buffer) {
	return _depth_buffer.data.expect<dx_depth_buffer_data>()->srv_bindless_idx;
}

void dx_depth_buffer_impl::dx_depth_buffer_destroy(depth_buffer& _depth_buffer, const device& _device) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* data = _depth_buffer.data.expect<dx_depth_buffer_data>();
	if (data->srv_bindless_idx != UINT32_MAX)
		dx_release_bindless_srv_slot(device_data, data->srv_bindless_idx);
	delete data;
	_depth_buffer = {};
}
} // namespace mars::graphics::dx
