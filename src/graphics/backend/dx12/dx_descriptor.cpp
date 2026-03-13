#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_descriptor.hpp>
#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
static const dx_root_param_entry* find_root_entry(
	const std::vector<dx_root_param_entry>& _layout,
	size_t _binding,
	mars_pipeline_descriptor_type _type
) {
	for (const auto& entry : _layout)
		if (entry.binding == _binding && entry.type == _type)
			return &entry;
	return nullptr;
}

static descriptor_set create_set_impl(
	const descriptor& _descriptor,
	const device& _device,
	const std::vector<dx_root_param_entry>& _layout,
	const std::vector<descriptor_set_create_params>& _params
) {
	auto set_data = new dx_descriptor_set_data();
	set_data->root_layout = &_layout;

	for (const auto& param_set : _params) {
		for (const auto& [buf, binding] : param_set.buffers) {
			auto buf_data = buf.data.expect<dx_buffer_data>();
			auto cbv_entry = find_root_entry(_layout, binding, MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			if (cbv_entry)
				set_data->cbv_bindings.push_back({cbv_entry->root_index, buf_data->resource->GetGPUVirtualAddress()});
		}
	}

	descriptor_set result;
	result.engine = _device.engine;
	result.data.store(set_data);
	return result;
}

descriptor dx_descriptor_impl::dx_descriptor_create(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight) {
	descriptor result;
	result.engine = _device.engine;
	result.data.store(new dx_descriptor_data());
	result.frames_in_flight = _frames_in_flight;
	return result;
}

descriptor_set dx_descriptor_impl::dx_descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	auto pipeline_data = _pipeline.data.expect<dx_pipeline_data>();
	return create_set_impl(_descriptor, _device, pipeline_data->root_layout, _params);
}

descriptor_set dx_descriptor_impl::dx_descriptor_set_create_compute(const descriptor& _descriptor, const device& _device, const compute_pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	auto pipeline_data = _pipeline.data.expect<dx_compute_pipeline_data>();
	return create_set_impl(_descriptor, _device, pipeline_data->root_layout, _params);
}

void dx_descriptor_impl::dx_descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame) {
	auto set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	for (auto& [root_index, gpu_addr] : set_data->cbv_bindings)
		cb_data->cmd_list->SetGraphicsRootConstantBufferView((UINT)root_index, gpu_addr);
}

void dx_descriptor_impl::dx_descriptor_set_bind_compute(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const compute_pipeline& _pipeline, size_t _current_frame) {
	auto set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto cb_data = _command_buffer.data.expect<dx_command_buffer_data>();
	for (auto& [root_index, gpu_addr] : set_data->cbv_bindings)
		cb_data->cmd_list->SetComputeRootConstantBufferView((UINT)root_index, gpu_addr);
}

void dx_descriptor_impl::dx_descriptor_set_update_cbv(descriptor_set& _descriptor_set, size_t _binding, const buffer& _buffer) {
	auto set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto buf_data = _buffer.data.expect<dx_buffer_data>();
	if (!set_data->root_layout) return;

	auto cbv_entry = find_root_entry(*set_data->root_layout, _binding, MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	if (!cbv_entry) return;

	D3D12_GPU_VIRTUAL_ADDRESS gpu_addr = buf_data->resource->GetGPUVirtualAddress();

	for (auto& [root_index, addr] : set_data->cbv_bindings) {
		if (root_index == cbv_entry->root_index) {
			addr = gpu_addr;
			return;
		}
	}
	set_data->cbv_bindings.push_back({cbv_entry->root_index, gpu_addr});
}

void dx_descriptor_impl::dx_descriptor_destroy(descriptor& _descriptor, const device& _device) {
	auto data = _descriptor.data.expect<dx_descriptor_data>();
	delete data;
	_descriptor = {};
}
} // namespace mars::graphics::dx
