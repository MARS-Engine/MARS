#include "dx_bindless_allocator.hpp"
#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_buffer.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
static D3D12_RESOURCE_STATES mars_buffer_state_to_dx12(mars_buffer_state state) {
	switch (state) {
	case MARS_BUFFER_STATE_COMMON:
		return D3D12_RESOURCE_STATE_COMMON;
	case MARS_BUFFER_STATE_SHADER_READ:
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	case MARS_BUFFER_STATE_UNORDERED_ACCESS:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case MARS_BUFFER_STATE_COPY_SOURCE:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case MARS_BUFFER_STATE_INDIRECT_ARGUMENT:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	default:
		return D3D12_RESOURCE_STATE_COMMON;
	}
}

buffer dx_buffer_impl::dx_buffer_create(const device& _device, const buffer_create_params& _params) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto data = new dx_buffer_data();
	data->size = _params.allocated_size;

	D3D12_HEAP_PROPERTIES heap_props = {};
	D3D12_RESOURCE_STATES initial_state;

	if (_params.buffer_property & MARS_BUFFER_PROPERTY_HOST_VISIBLE) {
		heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
		initial_state = D3D12_RESOURCE_STATE_GENERIC_READ;
	} else {
		heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
		initial_state = D3D12_RESOURCE_STATE_COMMON;
	}
	data->dx12_state = initial_state;

	const bool is_uav = (_params.buffer_type & MARS_BUFFER_TYPE_UNORDERED_ACCESS) != 0;

	D3D12_RESOURCE_DESC buf_desc = {};
	buf_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buf_desc.Width = _params.allocated_size;
	buf_desc.Height = 1;
	buf_desc.DepthOrArraySize = 1;
	buf_desc.MipLevels = 1;
	buf_desc.Format = DXGI_FORMAT_UNKNOWN;
	buf_desc.SampleDesc.Count = 1;
	buf_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	buf_desc.Flags = is_uav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

	device_data->device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &buf_desc, initial_state, nullptr, IID_PPV_ARGS(&data->resource));

	if (is_uav && data->resource) {
		const UINT slot = dx_allocate_bindless_uav_range(device_data, 1);
		data->uav_bindless_idx = slot;

		D3D12_CPU_DESCRIPTOR_HANDLE uav_cpu = device_data->bindless_heap->GetCPUDescriptorHandleForHeapStart();
		uav_cpu.ptr += (SIZE_T)slot * device_data->bindless_descriptor_size;

		const bool is_structured = (_params.buffer_type & MARS_BUFFER_TYPE_STRUCTURED) != 0;
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;
		if (is_structured && _params.stride > 0) {
			uav_desc.Format = DXGI_FORMAT_UNKNOWN;
			uav_desc.Buffer.StructureByteStride = static_cast<UINT>(_params.stride);
			uav_desc.Buffer.NumElements = static_cast<UINT>(_params.allocated_size / _params.stride);
			uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		} else {
			uav_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			uav_desc.Buffer.NumElements = static_cast<UINT>(_params.allocated_size / 4);
			uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		}
		device_data->device->CreateUnorderedAccessView(data->resource.Get(), nullptr, &uav_desc, uav_cpu);
	}

	const bool is_srv = (_params.buffer_type & MARS_BUFFER_TYPE_SHADER_RESOURCE) != 0;
	if (is_srv && data->resource) {
		const UINT slot = dx_allocate_bindless_srv_slot(device_data);
		data->srv_bindless_idx = slot;

		D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu = device_data->bindless_heap->GetCPUDescriptorHandleForHeapStart();
		srv_cpu.ptr += (SIZE_T)slot * device_data->bindless_descriptor_size;

		const bool is_structured = (_params.buffer_type & MARS_BUFFER_TYPE_STRUCTURED) != 0;
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		if (is_structured && _params.stride > 0) {
			srv_desc.Format = DXGI_FORMAT_UNKNOWN;
			srv_desc.Buffer.StructureByteStride = static_cast<UINT>(_params.stride);
			srv_desc.Buffer.NumElements = static_cast<UINT>(_params.allocated_size / _params.stride);
			srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		} else {
			srv_desc.Format = DXGI_FORMAT_R32_TYPELESS;
			srv_desc.Buffer.NumElements = static_cast<UINT>(_params.allocated_size / 4);
			srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		}
		device_data->device->CreateShaderResourceView(data->resource.Get(), &srv_desc, srv_cpu);
	}

	if (_params.buffer_type & MARS_BUFFER_TYPE_VERTEX) {
		data->vb_view.BufferLocation = data->resource->GetGPUVirtualAddress();
		data->vb_view.SizeInBytes = (UINT)_params.allocated_size;
		data->vb_view.StrideInBytes = (UINT)_params.stride;
	}

	buffer result;
	result.engine = _device.engine;
	result.data.store(data);
	result.allocated_size = _params.allocated_size;
	return result;
}

void dx_buffer_impl::dx_buffer_bind(buffer& _buffer, const command_buffer& _command_buffer) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	cb_data->cmd_list->IASetVertexBuffers(0, 1, &data->vb_view);
}

void dx_buffer_impl::dx_buffer_bind_index(buffer& _buffer, const command_buffer& _command_buffer) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	D3D12_INDEX_BUFFER_VIEW ib_view = {};
	ib_view.BufferLocation = data->resource->GetGPUVirtualAddress();
	ib_view.SizeInBytes = (UINT)data->size;
	ib_view.Format = DXGI_FORMAT_R32_UINT;
	cb_data->cmd_list->IASetIndexBuffer(&ib_view);
}

void dx_buffer_impl::dx_buffer_copy(buffer& _buffer, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) {
	auto dst = _buffer.data.expect<dx_buffer_data>();
	auto src = _src_buffer.data.expect<dx_buffer_data>();
	auto cb = _command_buffer.data.expect<dx_command_buffer_data>();
	cb->cmd_list->CopyBufferRegion(dst->resource.Get(), _offset, src->resource.Get(), 0, src->size);
}

void dx_buffer_impl::dx_buffer_transition(const command_buffer& _command_buffer, const buffer& _buffer, mars_buffer_state _state) {
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto data = _buffer.data.expect<dx_buffer_data>();

	const D3D12_RESOURCE_STATES target_state = mars_buffer_state_to_dx12(_state);
	if (data->dx12_state == target_state)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = data->resource.Get();
	barrier.Transition.StateBefore = data->dx12_state;
	barrier.Transition.StateAfter = target_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cb_data->cmd_list->ResourceBarrier(1, &barrier);
	data->dx12_state = target_state;
}

void* dx_buffer_impl::dx_buffer_map(buffer& _buffer, const device& _device, size_t _size, size_t _offset) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	void* mapped = nullptr;
	D3D12_RANGE read_range = {0, 0};
	data->resource->Map(0, &read_range, &mapped);
	return static_cast<uint8_t*>(mapped) + _offset;
}

void dx_buffer_impl::dx_buffer_unmap(buffer& _buffer, const device& _device) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	data->resource->Unmap(0, nullptr);
}

void dx_buffer_impl::dx_buffer_destroy(buffer& _buffer, const device& _device) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	auto* device_data = _device.data.expect<dx_device_data>();
	dx_release_bindless_uav_range(device_data, data->uav_bindless_idx, data->uav_bindless_idx == UINT32_MAX ? 0u : 1u);
	dx_release_bindless_srv_slot(device_data, data->srv_bindless_idx);
	delete data;
	_buffer = {};
}

uint32_t dx_buffer_impl::dx_buffer_get_uav_index(const buffer& _buffer) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	return static_cast<uint32_t>(data->uav_bindless_idx);
}

uint32_t dx_buffer_impl::dx_buffer_get_srv_index(const buffer& _buffer) {
	auto data = _buffer.data.expect<dx_buffer_data>();
	return static_cast<uint32_t>(data->srv_bindless_idx);
}
} // namespace mars::graphics::dx
