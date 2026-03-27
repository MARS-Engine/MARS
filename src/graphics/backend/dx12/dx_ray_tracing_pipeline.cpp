#include <mars/graphics/backend/dx12/dx_ray_tracing_pipeline.hpp>

#include "dx_internal.hpp"

#include <mars/graphics/functional/device.hpp>
#include <mars/utility/stringify.hpp>

#include <sstream>
#include <vector>
#include <wrl.h>

namespace mars::graphics::dx {
namespace {

std::string join_export_names(const std::vector<std::wstring>& _names) {
	std::ostringstream out;
	for (size_t index = 0u; index < _names.size(); ++index) {
		if (index != 0u)
			out << ", ";
		out << std::string(_names[index].begin(), _names[index].end());
	}
	return out.str();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> build_global_root_signature(dx_device_data* _device_data, const ray_tracing_pipeline_setup& _setup, std::vector<dx_root_param_entry>& _out_root_layout) {
	std::vector<D3D12_ROOT_PARAMETER1> params;
	_out_root_layout.clear();
	params.reserve(_setup.descriptors.size());
	_out_root_layout.reserve(_setup.descriptors.size());

	for (const auto& descriptor : _setup.descriptors) {
		if (descriptor.descriptor_type != MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			continue;

		D3D12_ROOT_PARAMETER1 cbv_param = {};
		cbv_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		cbv_param.Descriptor.ShaderRegister = static_cast<UINT>(descriptor.binding);
		cbv_param.Descriptor.RegisterSpace = static_cast<UINT>(descriptor.register_space);
		cbv_param.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
		cbv_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		_out_root_layout.push_back({
			.root_index = params.size(),
			.type = descriptor.descriptor_type,
			.binding = descriptor.binding,
		});
		params.push_back(cbv_param);
	}

	D3D12_ROOT_SIGNATURE_DESC1 root_signature_desc = {};
	root_signature_desc.NumParameters = static_cast<UINT>(params.size());
	root_signature_desc.pParameters = params.empty() ? nullptr : params.data();
	root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_desc = {};
	versioned_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	versioned_desc.Desc_1_1 = root_signature_desc;

	Microsoft::WRL::ComPtr<ID3DBlob> serialized;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	if (!dx_expect<D3D12SerializeVersionedRootSignature>(&versioned_desc, &serialized, &error)) {
		if (error && error->GetBufferPointer())
			mars::logger::error(dx12_log_channel(), "D3D12SerializeVersionedRootSignature failed: {}", static_cast<const char*>(error->GetBufferPointer()));
		return {};
	}

	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
	if (!dx_expect<&ID3D12Device::CreateRootSignature>(_device_data->device.Get(), 0u, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&root_signature)))
		return {};
	return root_signature;
}

} // namespace

ray_tracing_pipeline dx_ray_tracing_pipeline_impl::dx_ray_tracing_pipeline_create(const device& _device, const ray_tracing_pipeline_setup& _setup) {
	auto* device_data = _device.data.expect<dx_device_data>();

	auto* pipeline_data = new dx_rt_pipeline_data();
	pipeline_data->global_root_signature = build_global_root_signature(device_data, _setup, pipeline_data->root_layout);
	if (!pipeline_data->global_root_signature) {
		delete pipeline_data;
		return {};
	}

	struct shader_entry {
		ID3DBlob* blob = nullptr;
		std::wstring export_name;
		std::wstring original_name;
	};

	std::vector<shader_entry> shader_entries;

	auto* raygen_shader_data = _setup.raygen_shader.data.expect<dx_shader_data>();
	shader_entries.push_back({raygen_shader_data->compute_shader.Get(), L"RayGen", L"RayGen"});

	std::vector<std::wstring> miss_names;
	for (size_t index = 0u; index < _setup.miss_shaders.size(); ++index) {
		auto* shader_data = _setup.miss_shaders[index].data.expect<dx_shader_data>();
		miss_names.push_back(L"Miss" + std::to_wstring(index));
		shader_entries.push_back({shader_data->compute_shader.Get(), miss_names.back(), L"Miss"});
	}

	std::vector<std::wstring> closest_hit_names;
	for (size_t index = 0u; index < _setup.hit_groups.size(); ++index) {
		auto* shader_data = _setup.hit_groups[index].closest_hit_shader.data.expect<dx_shader_data>();
		closest_hit_names.push_back(L"ClosestHit" + std::to_wstring(index));
		shader_entries.push_back({shader_data->compute_shader.Get(), closest_hit_names.back(), L"ClosestHit"});
	}

	mars::logger::log(
		dx12_log_channel(),
		"Creating RT pipeline: descriptor_count={}, recursion_depth={}, payload_bytes={}, attribute_bytes={}, miss_count={}, hit_group_count={}",
		_setup.descriptors.size(),
		_setup.max_recursion_depth,
		_setup.max_payload_size,
		_setup.max_attribute_size,
		_setup.miss_shaders.size(),
		_setup.hit_groups.size()
	);
	mars::logger::log(
		dx12_log_channel(),
		"RT pipeline exports: raygen=RayGen, miss=[{}], closest_hit=[{}]",
		join_export_names(miss_names),
		join_export_names(closest_hit_names)
	);

	std::vector<D3D12_STATE_SUBOBJECT> subobjects;
	subobjects.reserve(shader_entries.size() * 2u + _setup.hit_groups.size() + 5u);

	std::vector<D3D12_DXIL_LIBRARY_DESC> library_descs;
	std::vector<D3D12_EXPORT_DESC> export_descs;
	library_descs.reserve(shader_entries.size());
	export_descs.reserve(shader_entries.size());

	for (auto& entry : shader_entries) {
		const wchar_t* rename_from = entry.original_name != entry.export_name ? entry.original_name.c_str() : nullptr;
		export_descs.push_back({entry.export_name.c_str(), rename_from, D3D12_EXPORT_FLAG_NONE});

		D3D12_DXIL_LIBRARY_DESC library_desc = {};
		library_desc.DXILLibrary.pShaderBytecode = entry.blob->GetBufferPointer();
		library_desc.DXILLibrary.BytecodeLength = entry.blob->GetBufferSize();
		library_desc.NumExports = 1u;
		library_desc.pExports = &export_descs.back();
		library_descs.push_back(library_desc);

		D3D12_STATE_SUBOBJECT subobject = {};
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		subobject.pDesc = &library_descs.back();
		subobjects.push_back(subobject);
	}

	std::vector<D3D12_HIT_GROUP_DESC> hit_group_descs;
	std::vector<std::wstring> hit_group_names;
	hit_group_descs.reserve(_setup.hit_groups.size());
	hit_group_names.reserve(_setup.hit_groups.size());

	for (size_t index = 0u; index < _setup.hit_groups.size(); ++index) {
		hit_group_names.push_back(L"HitGroup" + std::to_wstring(index));

		D3D12_HIT_GROUP_DESC hit_group_desc = {};
		hit_group_desc.HitGroupExport = hit_group_names.back().c_str();
		hit_group_desc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hit_group_desc.ClosestHitShaderImport = closest_hit_names[index].c_str();
		hit_group_desc.AnyHitShaderImport = nullptr;
		hit_group_desc.IntersectionShaderImport = nullptr;
		hit_group_descs.push_back(hit_group_desc);

		D3D12_STATE_SUBOBJECT subobject = {};
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		subobject.pDesc = &hit_group_descs.back();
		subobjects.push_back(subobject);
	}

	D3D12_RAYTRACING_SHADER_CONFIG shader_config = {};
	shader_config.MaxPayloadSizeInBytes = _setup.max_payload_size;
	shader_config.MaxAttributeSizeInBytes = _setup.max_attribute_size;
	{
		D3D12_STATE_SUBOBJECT subobject = {};
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		subobject.pDesc = &shader_config;
		subobjects.push_back(subobject);
	}

	D3D12_RAYTRACING_PIPELINE_CONFIG pipeline_config = {};
	pipeline_config.MaxTraceRecursionDepth = _setup.max_recursion_depth;
	{
		D3D12_STATE_SUBOBJECT subobject = {};
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		subobject.pDesc = &pipeline_config;
		subobjects.push_back(subobject);
	}

	ID3D12RootSignature* raw_root_signature = pipeline_data->global_root_signature.Get();
	{
		D3D12_STATE_SUBOBJECT subobject = {};
		subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		subobject.pDesc = &raw_root_signature;
		subobjects.push_back(subobject);
	}

	D3D12_STATE_OBJECT_DESC state_object_desc = {};
	state_object_desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	state_object_desc.NumSubobjects = static_cast<UINT>(subobjects.size());
	state_object_desc.pSubobjects = subobjects.data();

	if (!dx_expect<&ID3D12Device5::CreateStateObject>(device_data->device5.Get(), &state_object_desc, IID_PPV_ARGS(&pipeline_data->state_object))) {
		mars::logger::error(
			dx12_log_channel(),
			"CreateStateObject failed. descriptor_count={}, recursion_depth={}, payload_bytes={}, attribute_bytes={}, miss_count={}, hit_group_count={}",
			_setup.descriptors.size(),
			_setup.max_recursion_depth,
			_setup.max_payload_size,
			_setup.max_attribute_size,
			_setup.miss_shaders.size(),
			_setup.hit_groups.size()
		);
		delete pipeline_data;
		return {};
	}

	if (!pipeline_data->state_object) {
		mars::logger::error(dx12_log_channel(), "CreateStateObject returned success but the RT state object is null.");
		delete pipeline_data;
		return {};
	}

	pipeline_data->miss_group_count = static_cast<uint32_t>(_setup.miss_shaders.size());
	pipeline_data->hit_group_count = static_cast<uint32_t>(_setup.hit_groups.size());

	ray_tracing_pipeline result = {};
	result.engine = _device.engine;
	result.data.store(pipeline_data);
	return result;
}

void dx_ray_tracing_pipeline_impl::dx_ray_tracing_pipeline_bind(const ray_tracing_pipeline& _pipeline, const command_buffer& _command_buffer) {
	auto* pipeline_data = _pipeline.data.expect<dx_rt_pipeline_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	if (!pipeline_data->state_object) {
		mars::logger::error(dx12_log_channel(), "Attempted to bind RT pipeline with null state object");
		return;
	}
	assert(command_buffer_data->bindless_heap_raw && "bindless heap must be initialised on command buffer");

	command_buffer_data->cmd_list4->SetPipelineState1(pipeline_data->state_object.Get());

	ID3D12DescriptorHeap* heaps[] = {command_buffer_data->bindless_heap_raw};
	command_buffer_data->cmd_list->SetDescriptorHeaps(1u, heaps);
	command_buffer_data->cmd_list->SetComputeRootSignature(pipeline_data->global_root_signature.Get());
}

void dx_ray_tracing_pipeline_impl::dx_ray_tracing_pipeline_destroy(ray_tracing_pipeline& _pipeline, const device&) {
	auto* pipeline_data = _pipeline.data.expect<dx_rt_pipeline_data>();
	delete pipeline_data;
	_pipeline = {};
}

void dx_ray_tracing_pipeline_impl::dx_rt_pipeline_write_shader_identifiers(const ray_tracing_pipeline& _pipeline, const device&, uint32_t _first_group, uint32_t _count, void* _dst, uint32_t _dst_stride) {
	auto* pipeline_data = _pipeline.data.expect<dx_rt_pipeline_data>();

	Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> state_object_properties;
	pipeline_data->state_object->QueryInterface(IID_PPV_ARGS(&state_object_properties));

	const uint32_t miss_start = 1u;
	const uint32_t hit_start = miss_start + pipeline_data->miss_group_count;

	auto* dst = static_cast<uint8_t*>(_dst);
	for (uint32_t index = 0u; index < _count; ++index) {
		const uint32_t group_index = _first_group + index;
		std::wstring name;
		if (group_index == 0u)
			name = L"RayGen";
		else if (group_index < hit_start)
			name = L"Miss" + std::to_wstring(group_index - miss_start);
		else
			name = L"HitGroup" + std::to_wstring(group_index - hit_start);

		const void* identifier = state_object_properties->GetShaderIdentifier(name.c_str());
		std::memcpy(dst + index * _dst_stride, identifier, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}
}

} // namespace mars::graphics::dx
