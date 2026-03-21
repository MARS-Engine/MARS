#include "dx_bindless_allocator.hpp"
#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_texture.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
texture dx_texture_impl::dx_texture_create(const device& _device, const texture_create_params& _params) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto data = new dx_texture_data();
	data->format = dx_format_from_mars(_params.format);
	data->texture_type = _params.texture_type;
	data->mip_levels = _params.mip_levels;
	data->array_size = _params.array_size;

	D3D12_RESOURCE_DESC tex_desc = {};
	tex_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	tex_desc.Width = (UINT64)_params.size.x;
	tex_desc.Height = (UINT)_params.size.y;
	tex_desc.DepthOrArraySize = (UINT16)(_params.texture_type == MARS_TEXTURE_TYPE_CUBE ? 6 * _params.array_size : _params.array_size);
	tex_desc.MipLevels = (UINT16)_params.mip_levels;
	tex_desc.Format = data->format;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	if ((_params.usage & MARS_TEXTURE_USAGE_STORAGE) == MARS_TEXTURE_USAGE_STORAGE)
		tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	if ((_params.usage & MARS_TEXTURE_USAGE_COLOR_ATTACHMENT) == MARS_TEXTURE_USAGE_COLOR_ATTACHMENT)
		tex_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES default_heap = {};
	default_heap.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_STATES initial_state = ((_params.usage & MARS_TEXTURE_USAGE_TRANSFER_DST) == MARS_TEXTURE_USAGE_TRANSFER_DST) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON;
	data->dx12_state = initial_state;

	D3D12_CLEAR_VALUE rt_clear_value = {};
	D3D12_CLEAR_VALUE* p_clear_value = nullptr;
	if ((tex_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) &&
		!(tex_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)) {
		rt_clear_value.Format = data->format;
		rt_clear_value.Color[0] = _params.clear_color.x;
		rt_clear_value.Color[1] = _params.clear_color.y;
		rt_clear_value.Color[2] = _params.clear_color.z;
		rt_clear_value.Color[3] = _params.clear_color.w;
		p_clear_value = &rt_clear_value;
	}

	device_data->device->CreateCommittedResource(&default_heap, D3D12_HEAP_FLAG_NONE, &tex_desc, initial_state, p_clear_value, IID_PPV_ARGS(&data->resource));

	if ((_params.usage & MARS_TEXTURE_USAGE_TRANSFER_DST) == MARS_TEXTURE_USAGE_TRANSFER_DST) {
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		UINT num_rows;
		UINT64 row_size;
		UINT64 total_bytes;
		device_data->device->GetCopyableFootprints(&tex_desc, 0, 1, 0, &footprint, &num_rows, &row_size, &total_bytes);

		D3D12_RESOURCE_DESC upload_desc = {};
		upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		upload_desc.Width = total_bytes;
		upload_desc.Height = 1;
		upload_desc.DepthOrArraySize = 1;
		upload_desc.MipLevels = 1;
		upload_desc.Format = DXGI_FORMAT_UNKNOWN;
		upload_desc.SampleDesc.Count = 1;
		upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		D3D12_HEAP_PROPERTIES upload_heap = {};
		upload_heap.Type = D3D12_HEAP_TYPE_UPLOAD;

		device_data->device->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE, &upload_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&data->upload_resource));
	}

	assert(device_data->bindless_heap && "global bindless heap must be created before any texture");

	const UINT srv_slot = dx_allocate_bindless_srv_slot(device_data);
	D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu = device_data->bindless_heap->GetCPUDescriptorHandleForHeapStart();
	srv_cpu.ptr += (SIZE_T)srv_slot * device_data->bindless_descriptor_size;
	data->srv_bindless_idx = srv_slot;

	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.Format = data->format;
	if (data->texture_type == MARS_TEXTURE_TYPE_CUBE) {
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srv_desc.TextureCube.MostDetailedMip = 0;
		srv_desc.TextureCube.MipLevels = (UINT)data->mip_levels;
		srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
	} else if (data->array_size > 1) {
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srv_desc.Texture2DArray.MostDetailedMip = 0;
		srv_desc.Texture2DArray.MipLevels = (UINT)std::max<size_t>(1, data->mip_levels);
		srv_desc.Texture2DArray.FirstArraySlice = 0;
		srv_desc.Texture2DArray.ArraySize = (UINT)data->array_size;
		srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	} else {
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = (UINT)std::max<size_t>(1, data->mip_levels);
		srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	
	device_data->device->CreateShaderResourceView(data->resource.Get(), &srv_desc, srv_cpu);

	UINT uav_base = UINT32_MAX;
	if ((_params.usage & MARS_TEXTURE_USAGE_STORAGE) == MARS_TEXTURE_USAGE_STORAGE) {
		const UINT total_slices = (data->texture_type == MARS_TEXTURE_TYPE_CUBE)
									  ? 6u * (UINT)data->array_size
									  : (UINT)data->array_size;
		const UINT total_uav_descriptors = (UINT)data->mip_levels * total_slices;
		uav_base = dx_allocate_bindless_uav_range(device_data, total_uav_descriptors);
		data->uav_bindless_base = uav_base;
		data->uav_descriptor_count = total_uav_descriptors;

		for (UINT mip = 0; mip < (UINT)data->mip_levels; ++mip) {
			for (UINT slice = 0; slice < total_slices; ++slice) {
				const UINT uav_slot = uav_base + mip * total_slices + slice;
				D3D12_CPU_DESCRIPTOR_HANDLE uav_cpu = device_data->bindless_heap->GetCPUDescriptorHandleForHeapStart();
				uav_cpu.ptr += (SIZE_T)uav_slot * device_data->bindless_descriptor_size;

				D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
				uav_desc.Format = data->format;
				if (data->texture_type == MARS_TEXTURE_TYPE_CUBE || total_slices > 1) {
					uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uav_desc.Texture2DArray.MipSlice = mip;
					uav_desc.Texture2DArray.FirstArraySlice = slice;
					uav_desc.Texture2DArray.ArraySize = 1;
					uav_desc.Texture2DArray.PlaneSlice = 0;
				} else {
					uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uav_desc.Texture2D.MipSlice = mip;
					uav_desc.Texture2D.PlaneSlice = 0;
				}
				device_data->device->CreateUnorderedAccessView(data->resource.Get(), nullptr, &uav_desc, uav_cpu);
			}
		}
	}

	texture result;
	result.engine = _device.engine;
	result.data.store(data);
	result.view.data.store(data);
	result.size = _params.size;
	result.channels = 4;
	result.format_size = (_params.format == MARS_FORMAT_RGBA16_SFLOAT) ? 2 : 1;
	return result;
}

static D3D12_RESOURCE_STATES texture_state_to_dx12(mars_texture_state state) {
	switch (state) {
	case MARS_TEXTURE_STATE_COPY_DST:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case MARS_TEXTURE_STATE_UNORDERED_ACCESS:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case MARS_TEXTURE_STATE_COMMON:
		return D3D12_RESOURCE_STATE_COMMON;
	case MARS_TEXTURE_STATE_SHADER_READ:
	default:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
}

void dx_texture_impl::dx_texture_transition(const command_buffer& _command_buffer, texture& _texture, mars_texture_state _before, mars_texture_state _after) {
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	auto tex_data = _texture.data.expect<dx_texture_data>();
	(void)_before;

	const D3D12_RESOURCE_STATES after_state = texture_state_to_dx12(_after);
	if (tex_data->dx12_state == after_state)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = tex_data->resource.Get();
	barrier.Transition.StateBefore = tex_data->dx12_state;
	barrier.Transition.StateAfter = after_state;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cb_data->cmd_list->ResourceBarrier(1, &barrier);
	tex_data->dx12_state = after_state;
}

void dx_texture_impl::dx_texture_copy(texture& _texture, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) {
	auto tex_data = _texture.data.expect<dx_texture_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!tex_data->upload_resource)
		return;

	D3D12_RESOURCE_DESC tex_desc = tex_data->resource->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT num_rows;
	UINT64 row_size;
	UINT64 total_bytes;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	tex_data->resource->GetDevice(IID_PPV_ARGS(&device));
	device->GetCopyableFootprints(&tex_desc, 0, 1, 0, &footprint, &num_rows, &row_size, &total_bytes);

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	dst.pResource = tex_data->resource.Get();
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION src = {};
	src.pResource = tex_data->upload_resource.Get();
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint = footprint;
	src.PlacedFootprint.Offset += _offset;

	cb_data->cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
}

void* dx_texture_impl::dx_texture_map(texture& _texture, const device& _device) {
	auto data = _texture.data.expect<dx_texture_data>();
	if (!data->upload_resource)
		return nullptr;
	void* mapped_data = nullptr;
	HRESULT hr = data->upload_resource->Map(0, nullptr, &mapped_data);
	if (FAILED(hr))
		return nullptr;
	return mapped_data;
}

void dx_texture_impl::dx_texture_unmap(texture& _texture, const device& _device) {
	auto data = _texture.data.expect<dx_texture_data>();
	if (!data->upload_resource)
		return;
	data->upload_resource->Unmap(0, nullptr);
}

texture_upload_layout dx_texture_impl::dx_texture_get_upload_layout(texture& _texture, const device& _device) {
	auto tex_data = _texture.data.expect<dx_texture_data>();
	auto device_data = _device.data.expect<dx_device_data>();

	texture_upload_layout layout = {};
	if (!tex_data->upload_resource)
		return layout;

	D3D12_RESOURCE_DESC tex_desc = tex_data->resource->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT num_rows;
	UINT64 row_size;
	UINT64 total_bytes;
	device_data->device->GetCopyableFootprints(&tex_desc, 0, 1, 0, &footprint, &num_rows, &row_size, &total_bytes);

	layout.row_count = num_rows;
	layout.row_size = (size_t)row_size;
	layout.row_pitch = (size_t)footprint.Footprint.RowPitch;
	layout.offset = (size_t)footprint.Offset;
	return layout;
}

uint32_t dx_texture_impl::dx_texture_get_srv_index(const texture& _texture) {
	auto data = _texture.data.expect<dx_texture_data>();
	return data->srv_bindless_idx;
}

uint32_t dx_texture_impl::dx_texture_get_uav_base(const texture& _texture) {
	auto data = _texture.data.expect<dx_texture_data>();
	return data->uav_bindless_base;
}

void dx_texture_impl::dx_texture_destroy(texture& _texture, const device& _device) {
	auto data = _texture.data.expect<dx_texture_data>();
	auto* device_data = _device.data.expect<dx_device_data>();
	dx_release_bindless_srv_slot(device_data, data->srv_bindless_idx);
	dx_release_bindless_uav_range(device_data, data->uav_bindless_base, data->uav_descriptor_count);
	delete data;
	_texture = {};
}
} // namespace mars::graphics::dx
