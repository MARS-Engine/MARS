#pragma once

#include "dx_internal.hpp"

#include <mars/debug/logger.hpp>

#include <vector>

namespace mars::graphics::dx {

inline log_channel& dx_root_signature_channel() {
	static log_channel channel("dx12");
	return channel;
}

template <typename SetupT, typename PipelineDataT>
inline bool dx_build_root_signature(ID3D12Device* device, const SetupT& setup, PipelineDataT* out_data, D3D12_ROOT_SIGNATURE_FLAGS flags, const char* label) {
	std::vector<D3D12_ROOT_PARAMETER1> root_params;

	for (const auto& desc : setup.descriptors) {
		if (desc.descriptor_type != MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			continue;

		D3D12_ROOT_PARAMETER1 param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.Descriptor.ShaderRegister = static_cast<UINT>(desc.binding);
		param.Descriptor.RegisterSpace = 0;
		param.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		out_data->root_layout.push_back({root_params.size(), desc.descriptor_type, desc.binding});
		root_params.push_back(param);
	}

	if (setup.push_constant_count > 0) {
		D3D12_ROOT_PARAMETER1 push_param = {};
		push_param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		push_param.Constants.ShaderRegister = 15;
		push_param.Constants.RegisterSpace = 0;
		push_param.Constants.Num32BitValues = static_cast<UINT>(setup.push_constant_count);
		push_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		out_data->push_constants_root_index = root_params.size();
		out_data->push_constant_count = setup.push_constant_count;
		out_data->has_push_constants = true;
		root_params.push_back(push_param);
	}

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC1 root_sig_desc = {};
	root_sig_desc.NumParameters = static_cast<UINT>(root_params.size());
	root_sig_desc.pParameters = root_params.empty() ? nullptr : root_params.data();
	root_sig_desc.NumStaticSamplers = 1;
	root_sig_desc.pStaticSamplers = &sampler;
	root_sig_desc.Flags = flags;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_desc = {};
	versioned_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	versioned_desc.Desc_1_1 = root_sig_desc;

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	if (FAILED(dx_expect<D3D12SerializeVersionedRootSignature>(&versioned_desc, &signature, &error))) {
		if (error)
			logger::error(dx_root_signature_channel(), "{} root signature serialization failed: {}", label, static_cast<const char*>(error->GetBufferPointer()));
		return false;
	}

	const HRESULT hr = dx_expect<&ID3D12Device::CreateRootSignature>(device, 0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&out_data->root_signature));
	return SUCCEEDED(hr);
}

} // namespace mars::graphics::dx
