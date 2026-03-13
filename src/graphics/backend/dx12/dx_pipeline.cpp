#include "dx_internal.hpp"
#include "dx_root_signature_utils.hpp"
#include <mars/graphics/backend/dx12/dx_pipeline.hpp>
#include <mars/graphics/functional/device.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

static D3D12_COMPARISON_FUNC mars_compare_op_to_dx12(mars_compare_op op) {
	switch (op) {
	case MARS_COMPARE_OP_NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case MARS_COMPARE_OP_LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case MARS_COMPARE_OP_EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case MARS_COMPARE_OP_LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case MARS_COMPARE_OP_GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	case MARS_COMPARE_OP_NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case MARS_COMPARE_OP_GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case MARS_COMPARE_OP_ALWAYS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	default:
		return D3D12_COMPARISON_FUNC_LESS;
	}
}

static void dump_dx12_info_queue(ID3D12Device* device) {
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;
	if (FAILED(device->QueryInterface(IID_PPV_ARGS(&info_queue))))
		return;

	UINT64 count = info_queue->GetNumStoredMessages();
	for (UINT64 i = 0; i < count; ++i) {
		SIZE_T msg_size = 0;
		info_queue->GetMessage(i, nullptr, &msg_size);
		if (msg_size == 0)
			continue;

		auto* msg = (D3D12_MESSAGE*)malloc(msg_size);
		if (!msg)
			continue;
		info_queue->GetMessage(i, msg, &msg_size);
		logger::error(dx12_channel, "D3D12: {}", msg->pDescription);
		free(msg);
	}
	info_queue->ClearStoredMessages();
}

static DXGI_FORMAT mars_format_to_dxgi(mars_format_type fmt) {
	switch (fmt) {
	case MARS_FORMAT_R32_SFLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case MARS_FORMAT_RG32_SFLOAT:
		return DXGI_FORMAT_R32G32_FLOAT;
	case MARS_FORMAT_RGB32_SFLOAT:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case MARS_FORMAT_RGBA32_SFLOAT:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case MARS_FORMAT_R32_UINT:
		return DXGI_FORMAT_R32_UINT;
	case MARS_FORMAT_RG32_UINT:
		return DXGI_FORMAT_R32G32_UINT;
	case MARS_FORMAT_RGB32_UINT:
		return DXGI_FORMAT_R32G32B32_UINT;
	case MARS_FORMAT_RGBA32_UINT:
		return DXGI_FORMAT_R32G32B32A32_UINT;
	case MARS_FORMAT_RGBA16_SFLOAT:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case MARS_FORMAT_RGBA8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case MARS_FORMAT_RGBA8_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case MARS_FORMAT_D32_SFLOAT:
		return DXGI_FORMAT_D32_FLOAT;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

static D3D12_SHADER_VISIBILITY stage_to_visibility(mars_pipeline_stage stage) {
	switch (stage) {
	case MARS_PIPELINE_STAGE_VERTEX:
	case MARS_PIPELINE_STAGE_FRAGMENT:
	case MARS_PIPELINE_STAGE_COMPUTE:
	default:
		return D3D12_SHADER_VISIBILITY_ALL;
	}
}

pipeline dx_pipeline_impl::dx_pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto shader_data = _setup.pipeline_shader.data.expect<dx_shader_data>();
	auto rp_data = _render_pass.data.expect<dx_render_pass_data>();
	auto data = new dx_pipeline_data();

	if (!shader_data->vertex_shader || !shader_data->pixel_shader) {
		logger::error(dx12_channel, "Graphics pipeline creation failed: missing VS/PS bytecode");
		pipeline result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	const bool root_signature_ok = dx_build_root_signature(
		device_data->device.Get(),
		_setup,
		data,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED,
		"graphics"
	);
	if (!root_signature_ok || !data->root_signature) {
		logger::error(dx12_channel, "Graphics pipeline creation failed: root signature creation failed");
		pipeline result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> input_elements;
	input_elements.reserve(_setup.attributes.size());
	for (size_t i = 0; i < _setup.attributes.size(); i++) {
		auto& attr = _setup.attributes[i];
		D3D12_INPUT_ELEMENT_DESC elem = {};
		elem.SemanticName = attr.semantic_name.c_str();
		elem.SemanticIndex = 0;
		elem.Format = mars_format_to_dxgi(attr.input_format);
		elem.InputSlot = (UINT)attr.binding;
		elem.AlignedByteOffset = (UINT)attr.offset;
		elem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elem.InstanceDataStepRate = 0;
		input_elements.push_back(elem);
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.InputLayout = {input_elements.empty() ? nullptr : input_elements.data(), (UINT)input_elements.size()};
	pso_desc.pRootSignature = data->root_signature.Get();
	pso_desc.VS = {shader_data->vertex_shader->GetBufferPointer(), shader_data->vertex_shader->GetBufferSize()};
	pso_desc.PS = {shader_data->pixel_shader->GetBufferPointer(), shader_data->pixel_shader->GetBufferSize()};

	pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pso_desc.RasterizerState.FrontCounterClockwise = FALSE;
	pso_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pso_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pso_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pso_desc.RasterizerState.DepthClipEnable = TRUE;
	pso_desc.RasterizerState.MultisampleEnable = FALSE;
	pso_desc.RasterizerState.AntialiasedLineEnable = FALSE;
	pso_desc.RasterizerState.ForcedSampleCount = 0;
	pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	pso_desc.BlendState.AlphaToCoverageEnable = FALSE;
	pso_desc.BlendState.IndependentBlendEnable = FALSE;
	for (auto& rt : pso_desc.BlendState.RenderTarget) {
		rt.BlendEnable = FALSE;
		rt.LogicOpEnable = FALSE;
		rt.SrcBlend = D3D12_BLEND_ONE;
		rt.DestBlend = D3D12_BLEND_ZERO;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_ZERO;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.LogicOp = D3D12_LOGIC_OP_NOOP;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	pso_desc.DepthStencilState.DepthEnable = FALSE;
	pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	pso_desc.DepthStencilState.StencilEnable = FALSE;
	pso_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	pso_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	pso_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	pso_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	pso_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	pso_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pso_desc.DepthStencilState.BackFace = pso_desc.DepthStencilState.FrontFace;
	pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pso_desc.StreamOutput.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

	if (rp_data->depth_format != MARS_FORMAT_UNDEFINED) {
		pso_desc.DepthStencilState.DepthEnable = TRUE;
		pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		pso_desc.DSVFormat = mars_format_to_dxgi(rp_data->depth_format);
	}

	if (_setup.has_depth_test_override)
		pso_desc.DepthStencilState.DepthEnable = _setup.depth_test_enable ? TRUE : FALSE;
	if (_setup.has_depth_write_override)
		pso_desc.DepthStencilState.DepthWriteMask =
			_setup.depth_write_enable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	if (_setup.has_depth_compare_override)
		pso_desc.DepthStencilState.DepthFunc = mars_compare_op_to_dx12(_setup.depth_compare);
	if (_setup.has_alpha_blend_override && _setup.alpha_blend_enable) {
		auto& rt = pso_desc.BlendState.RenderTarget[0];
		rt.BlendEnable = TRUE;
		rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	}

	pso_desc.SampleMask = UINT_MAX;
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = mars_format_to_dxgi(rp_data->format);
	pso_desc.SampleDesc.Count = 1;
	pso_desc.SampleDesc.Quality = 0;
	pso_desc.NodeMask = 0;
	pso_desc.CachedPSO.pCachedBlob = nullptr;
	pso_desc.CachedPSO.CachedBlobSizeInBytes = 0;
	pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hr = dx_expect<&ID3D12Device::CreateGraphicsPipelineState>(device_data->device.Get(), &pso_desc, IID_PPV_ARGS(&data->pipeline_state));
	if (FAILED(hr)) {
		dump_dx12_info_queue(device_data->device.Get());
		logger::error(dx12_channel, "PSO diagnostics: rootSig={} VS={} ({}) PS={} ({}) RTVFormat={} DSVFormat={} NumRT={}", (void*)data->root_signature.Get(), (void*)shader_data->vertex_shader.Get(), shader_data->vertex_shader_path, (void*)shader_data->pixel_shader.Get(), shader_data->pixel_shader_path, (int)pso_desc.RTVFormats[0], (int)pso_desc.DSVFormat, (unsigned)pso_desc.NumRenderTargets);
	}

	pipeline result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void dx_pipeline_impl::dx_pipeline_bind(const pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params) {
	auto pipeline_data = _pipeline.data.expect<dx_pipeline_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!pipeline_data->root_signature || !pipeline_data->pipeline_state) {
		logger::error(dx12_channel, "Attempted to bind invalid graphics pipeline");
		return;
	}

	assert(cb_data->bindless_heap_raw && "bindless heap must be initialised on command buffer");
	ID3D12DescriptorHeap* heaps[] = {cb_data->bindless_heap_raw};
	cb_data->cmd_list->SetDescriptorHeaps(1, heaps);
	cb_data->cmd_list->SetGraphicsRootSignature(pipeline_data->root_signature.Get());
	cb_data->cmd_list->SetPipelineState(pipeline_data->pipeline_state.Get());

	D3D12_VIEWPORT viewport = {0.0f, 0.0f, (FLOAT)_params.size.x, (FLOAT)_params.size.y, 0.0f, 1.0f};
	D3D12_RECT scissor = {0, 0, (LONG)_params.size.x, (LONG)_params.size.y};
	cb_data->cmd_list->RSSetViewports(1, &viewport);
	cb_data->cmd_list->RSSetScissorRects(1, &scissor);
	cb_data->cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void dx_pipeline_impl::dx_pipeline_destroy(pipeline& _pipeline, const device& _device) {
	auto data = _pipeline.data.expect<dx_pipeline_data>();
	delete data;
	_pipeline = {};
}
} // namespace mars::graphics::dx
