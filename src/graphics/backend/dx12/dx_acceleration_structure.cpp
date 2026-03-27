#include <mars/graphics/backend/dx12/dx_acceleration_structure.hpp>

#include "dx_bindless_allocator.hpp"
#include "dx_internal.hpp"

#include <mars/graphics/functional/device.hpp>

#include <cstring>

namespace mars::graphics::dx {
namespace {

Microsoft::WRL::ComPtr<ID3D12Resource> dx_alloc_as_buffer(dx_device_data* _device_data, UINT64 _size, D3D12_RESOURCE_STATES _initial_state, D3D12_RESOURCE_FLAGS _flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
	D3D12_HEAP_PROPERTIES heap = {};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = _size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = _flags;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	if (!dx_expect<&ID3D12Device::CreateCommittedResource>(_device_data->device.Get(), &heap, D3D12_HEAP_FLAG_NONE, &desc, _initial_state, nullptr, IID_PPV_ARGS(&resource)))
		return {};

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> dx_alloc_upload_buffer(dx_device_data* _device_data, UINT64 _size) {
	D3D12_HEAP_PROPERTIES heap = {};
	heap.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = _size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	if (!dx_expect<&ID3D12Device::CreateCommittedResource>(_device_data->device.Get(), &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)))
		return {};

	return resource;
}

} // namespace

blas dx_acceleration_structure_impl::dx_blas_create(const device& _device, const command_buffer& _command_buffer, const blas_create_params& _params) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometry_descs;
	geometry_descs.reserve(_params.geometries.size());

	for (const auto& geometry : _params.geometries) {
		auto* vertex_buffer_data = geometry.vertex_buffer_handle.expect<dx_buffer_data>();
		auto* index_buffer_data = geometry.index_buffer_handle.expect<dx_buffer_data>();

		D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc = {};
		geometry_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometry_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometry_desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometry_desc.Triangles.VertexCount = geometry.vertex_count;
		geometry_desc.Triangles.VertexBuffer.StartAddress = vertex_buffer_data->resource->GetGPUVirtualAddress();
		geometry_desc.Triangles.VertexBuffer.StrideInBytes = geometry.vertex_stride;
		geometry_desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometry_desc.Triangles.IndexCount = geometry.index_count;
		geometry_desc.Triangles.IndexBuffer = index_buffer_data->resource->GetGPUVirtualAddress();
		geometry_descs.push_back(geometry_desc);
	}

	const auto build_flags = _params.allow_update ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = build_flags;
	inputs.NumDescs = static_cast<UINT>(geometry_descs.size());
	inputs.pGeometryDescs = geometry_descs.data();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
	device_data->device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

	auto* blas_data = new dx_blas_data();
	blas_data->size = prebuild.ResultDataMaxSizeInBytes;

	blas_data->result_buffer = dx_alloc_as_buffer(
		device_data, prebuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
	);
	blas_data->scratch_buffer = dx_alloc_as_buffer(
		device_data, prebuild.ScratchDataSizeInBytes,
		D3D12_RESOURCE_STATE_COMMON
	);
	if (!blas_data->result_buffer || !blas_data->scratch_buffer) {
		delete blas_data;
		return {};
	}
	blas_data->gpu_va = blas_data->result_buffer->GetGPUVirtualAddress();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.DestAccelerationStructureData = blas_data->gpu_va;
	build_desc.Inputs = inputs;
	build_desc.ScratchAccelerationStructureData = blas_data->scratch_buffer->GetGPUVirtualAddress();

	command_buffer_data->cmd_list4->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = blas_data->result_buffer.Get();
	command_buffer_data->cmd_list4->ResourceBarrier(1u, &barrier);

	blas result = {};
	result.engine = _device.engine;
	result.data.store(blas_data);
	result.size = prebuild.ResultDataMaxSizeInBytes;
	return result;
}

void dx_acceleration_structure_impl::dx_blas_update(blas& _blas, const command_buffer& _command_buffer, const blas_create_params& _params) {
	auto* blas_data = _blas.data.expect<dx_blas_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometry_descs;
	geometry_descs.reserve(_params.geometries.size());

	for (const auto& geometry : _params.geometries) {
		auto* vertex_buffer_data = geometry.vertex_buffer_handle.expect<dx_buffer_data>();
		auto* index_buffer_data = geometry.index_buffer_handle.expect<dx_buffer_data>();

		D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc = {};
		geometry_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometry_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometry_desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometry_desc.Triangles.VertexCount = geometry.vertex_count;
		geometry_desc.Triangles.VertexBuffer.StartAddress = vertex_buffer_data->resource->GetGPUVirtualAddress();
		geometry_desc.Triangles.VertexBuffer.StrideInBytes = geometry.vertex_stride;
		geometry_desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometry_desc.Triangles.IndexCount = geometry.index_count;
		geometry_desc.Triangles.IndexBuffer = index_buffer_data->resource->GetGPUVirtualAddress();
		geometry_descs.push_back(geometry_desc);
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	inputs.NumDescs = static_cast<UINT>(geometry_descs.size());
	inputs.pGeometryDescs = geometry_descs.data();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.DestAccelerationStructureData = blas_data->gpu_va;
	build_desc.SourceAccelerationStructureData = blas_data->gpu_va;
	build_desc.Inputs = inputs;
	build_desc.ScratchAccelerationStructureData = blas_data->scratch_buffer->GetGPUVirtualAddress();

	command_buffer_data->cmd_list4->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = blas_data->result_buffer.Get();
	command_buffer_data->cmd_list4->ResourceBarrier(1u, &barrier);
}

void dx_acceleration_structure_impl::dx_blas_destroy(blas& _blas, const device& _device) {
	(void)_device;
	auto* blas_data = _blas.data.expect<dx_blas_data>();
	delete blas_data;
	_blas = {};
}

tlas dx_acceleration_structure_impl::dx_tlas_create(const device& _device, const tlas_create_params& _params) {
	auto* device_data = _device.data.expect<dx_device_data>();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = _params.allow_update
		? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE
		: D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = _params.max_instance_count;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild = {};
	device_data->device5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

	auto* tlas_data = new dx_tlas_data();
	tlas_data->size = prebuild.ResultDataMaxSizeInBytes;
	tlas_data->max_instance_count = _params.max_instance_count;
	tlas_data->allow_update = _params.allow_update;

	tlas_data->result_buffer = dx_alloc_as_buffer(
		device_data, prebuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE
	);

	tlas_data->scratch_buffer = dx_alloc_as_buffer(
		device_data, prebuild.ScratchDataSizeInBytes,
		D3D12_RESOURCE_STATE_COMMON
	);

	const UINT64 instance_buf_size = _params.max_instance_count * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	tlas_data->instance_buffer = dx_alloc_upload_buffer(device_data, instance_buf_size);
	if (!tlas_data->result_buffer || !tlas_data->scratch_buffer || !tlas_data->instance_buffer) {
		delete tlas_data;
		return {};
	}
	tlas_data->gpu_va = tlas_data->result_buffer->GetGPUVirtualAddress();

	D3D12_RANGE read_range = {};
	if (!dx_expect<&ID3D12Resource::Map>(tlas_data->instance_buffer.Get(), 0u, &read_range, &tlas_data->instance_mapped)) {
		delete tlas_data;
		return {};
	}

	tlas_data->srv_bindless_idx = dx_allocate_bindless_srv_slot(device_data);
	D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
	srv_desc.Format = DXGI_FORMAT_UNKNOWN;
	srv_desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srv_desc.RaytracingAccelerationStructure.Location = tlas_data->gpu_va;

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle = device_data->bindless_heap->GetCPUDescriptorHandleForHeapStart();
	cpu_handle.ptr += tlas_data->srv_bindless_idx * device_data->bindless_descriptor_size;
	device_data->device->CreateShaderResourceView(nullptr, &srv_desc, cpu_handle);

	tlas result = {};
	result.engine = _device.engine;
	result.data.store(tlas_data);
	result.size = prebuild.ResultDataMaxSizeInBytes;
	return result;
}

void dx_acceleration_structure_impl::dx_tlas_build(tlas& _tlas, const command_buffer& _command_buffer, const acceleration_structure_instance* _instances, uint32_t _count) {
	auto* tlas_data = _tlas.data.expect<dx_tlas_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	auto* instance_descs = static_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(tlas_data->instance_mapped);
	for (uint32_t i = 0u; i < _count; ++i) {
		const auto& src = _instances[i];
		D3D12_RAYTRACING_INSTANCE_DESC& instance_desc = instance_descs[i];
		static_assert(sizeof(instance_desc.Transform) == sizeof(row_matrix3x4<float>));
		std::memcpy(instance_desc.Transform, &src.transform, sizeof(instance_desc.Transform));
		instance_desc.InstanceID = src.instance_id & 0x00FFFFFFu;
		instance_desc.InstanceMask = src.mask;
		instance_desc.InstanceContributionToHitGroupIndex = src.sbt_record_offset;
		instance_desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		auto* blas_data = src.bottom_level->data.expect<dx_blas_data>();
		instance_desc.AccelerationStructure = blas_data->gpu_va;
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
	build_desc.DestAccelerationStructureData = tlas_data->gpu_va;
	build_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	build_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	if (tlas_data->allow_update) {
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		if (tlas_data->has_been_built) {
			build_desc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
			build_desc.SourceAccelerationStructureData = tlas_data->gpu_va;
		}
	}
	else
		build_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	
	build_desc.Inputs.NumDescs = _count;
	build_desc.Inputs.InstanceDescs = tlas_data->instance_buffer->GetGPUVirtualAddress();
	build_desc.ScratchAccelerationStructureData = tlas_data->scratch_buffer->GetGPUVirtualAddress();

	command_buffer_data->cmd_list4->BuildRaytracingAccelerationStructure(&build_desc, 0u, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = tlas_data->result_buffer.Get();
	command_buffer_data->cmd_list4->ResourceBarrier(1u, &barrier);
	tlas_data->has_been_built = true;
}

uint32_t dx_acceleration_structure_impl::dx_tlas_get_srv_index(const tlas& _tlas) {
	return _tlas.data.expect<dx_tlas_data>()->srv_bindless_idx;
}

void dx_acceleration_structure_impl::dx_tlas_destroy(tlas& _tlas, const device& _device) {
	auto* tlas_data = _tlas.data.expect<dx_tlas_data>();
	if (tlas_data->instance_mapped)
		tlas_data->instance_buffer->Unmap(0u, nullptr);
	auto* device_data = _device.data.expect<dx_device_data>();
	dx_release_bindless_srv_slot(device_data, tlas_data->srv_bindless_idx);
	delete tlas_data;
	_tlas = {};
}

} // namespace mars::graphics::dx
