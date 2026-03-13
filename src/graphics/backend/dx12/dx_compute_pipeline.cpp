#include "dx_internal.hpp"
#include "dx_root_signature_utils.hpp"
#include <mars/graphics/backend/dx12/dx_compute_pipeline.hpp>
#include <mars/graphics/functional/device.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

compute_pipeline dx_compute_pipeline_impl::dx_compute_pipeline_create(const device& _device, const compute_pipeline_setup& _setup) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto shader_data = _setup.pipeline_shader.data.expect<dx_shader_data>();
	auto data = new dx_compute_pipeline_data();

	if (!shader_data->compute_shader) {
		logger::error(dx12_channel, "Compute pipeline creation failed: missing compute shader bytecode");
		compute_pipeline result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	const bool root_signature_ok = dx_build_root_signature(
		device_data->device.Get(),
		_setup,
		data,
		D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED,
		"compute"
	);
	if (!root_signature_ok || !data->root_signature) {
		logger::error(dx12_channel, "Compute pipeline creation failed: root signature creation failed");
		compute_pipeline result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = {};
	pso_desc.pRootSignature = data->root_signature.Get();
	pso_desc.CS = {shader_data->compute_shader->GetBufferPointer(), shader_data->compute_shader->GetBufferSize()};

	HRESULT hr = dx_expect<&ID3D12Device::CreateComputePipelineState>(device_data->device.Get(), &pso_desc, IID_PPV_ARGS(&data->pipeline_state));
	if (FAILED(hr))
		logger::error(dx12_channel, "Compute PSO diagnostics: rootSig={} CS={} ({})", (void*)data->root_signature.Get(), (void*)shader_data->compute_shader.Get(), shader_data->compute_shader_path);

	compute_pipeline result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void dx_compute_pipeline_impl::dx_compute_pipeline_bind(const compute_pipeline& _pipeline, const command_buffer& _command_buffer) {
	auto pipeline_data = _pipeline.data.expect<dx_compute_pipeline_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	if (!pipeline_data->root_signature || !pipeline_data->pipeline_state) {
		logger::error(dx12_channel, "Attempted to bind invalid compute pipeline");
		return;
	}

	assert(cb_data->bindless_heap_raw && "bindless heap must be initialised on command buffer");
	ID3D12DescriptorHeap* heaps[] = {cb_data->bindless_heap_raw};
	cb_data->cmd_list->SetDescriptorHeaps(1, heaps);
	cb_data->cmd_list->SetComputeRootSignature(pipeline_data->root_signature.Get());
	cb_data->cmd_list->SetPipelineState(pipeline_data->pipeline_state.Get());
}

void dx_compute_pipeline_impl::dx_compute_pipeline_destroy(compute_pipeline& _pipeline, const device& _device) {
	auto data = _pipeline.data.expect<dx_compute_pipeline_data>();
	delete data;
	_pipeline = {};
}
} // namespace mars::graphics::dx
