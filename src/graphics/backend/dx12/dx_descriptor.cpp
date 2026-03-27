#include <mars/graphics/backend/dx12/dx_descriptor.hpp>

#include "dx_internal.hpp"

#include <mars/graphics/functional/device.hpp>

namespace mars::graphics::dx {
namespace {

const dx_root_param_entry* find_root_entry(const std::vector<dx_root_param_entry>& _layout, size_t _binding, mars_pipeline_descriptor_type _type) {
	for (const auto& entry : _layout)
		if (entry.binding == _binding && entry.type == _type)
			return &entry;
	return nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE bindless_gpu_handle(const dx_device_data* _device_data, UINT _descriptor_index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = _device_data->bindless_heap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(_descriptor_index) * _device_data->bindless_descriptor_size;
	return handle;
}

UINT texture_layer_count(const dx_texture_data* _texture_data) {
	return _texture_data->texture_type == MARS_TEXTURE_TYPE_CUBE
		? static_cast<UINT>(_texture_data->array_size * 6u)
		: static_cast<UINT>(_texture_data->array_size);
}

UINT texture_uav_descriptor_index(const dx_texture_data* _texture_data, size_t _mip_level, size_t _array_slice) {
	const UINT total_slices = texture_layer_count(_texture_data);
	if (_texture_data->uav_bindless_base == UINT32_MAX || _mip_level >= _texture_data->mip_levels || _array_slice >= total_slices)
		return UINT32_MAX;

	return _texture_data->uav_bindless_base + static_cast<UINT>(_mip_level) * total_slices + static_cast<UINT>(_array_slice);
}

template <typename BindFn>
void bind_descriptor_tables(const dx_descriptor_set_data* _set_data, BindFn&& _bind_fn) {
	for (const auto& [root_index, handle] : _set_data->srv_bindings)
		_bind_fn(static_cast<UINT>(root_index), handle);
	for (const auto& [root_index, handle] : _set_data->uav_bindings)
		_bind_fn(static_cast<UINT>(root_index), handle);
}

descriptor_set create_set_impl(const descriptor&, const device& _device, const std::vector<dx_root_param_entry>& _layout, const std::vector<descriptor_set_create_params>& _params) {
	auto* device_data = _device.data.expect<dx_device_data>();
	auto* set_data = new dx_descriptor_set_data();
	set_data->root_layout = &_layout;

	for (const auto& param_set : _params) {
		for (const auto& [buffer_handle, binding] : param_set.buffers) {
			auto* buffer_data = buffer_handle.data.expect<dx_buffer_data>();
			const auto* cbv_entry = find_root_entry(_layout, binding, MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			if (cbv_entry)
				set_data->cbv_bindings.push_back({cbv_entry->root_index, buffer_data->resource->GetGPUVirtualAddress()});
		}

		for (const auto& texture_binding : param_set.textures) {
			auto* texture_data = texture_binding.image.data.get<dx_texture_data>();
			if (texture_data == nullptr)
				continue;

			const auto* root_entry = find_root_entry(_layout, texture_binding.binding, texture_binding.descriptor_type);
			if (root_entry == nullptr)
				continue;

			switch (texture_binding.descriptor_type) {
			case MARS_PIPELINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER:
				if (texture_data->srv_bindless_idx == UINT32_MAX)
					continue;
				set_data->srv_bindings.push_back({
					root_entry->root_index,
					bindless_gpu_handle(device_data, texture_data->srv_bindless_idx),
				});
				break;
			case MARS_PIPELINE_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
				const UINT descriptor_index = texture_uav_descriptor_index(texture_data, texture_binding.mip_level, texture_binding.array_slice);
				if (descriptor_index == UINT32_MAX)
					continue;
				set_data->uav_bindings.push_back({
					root_entry->root_index,
					bindless_gpu_handle(device_data, descriptor_index),
				});
				break;
			}
			default:
				break;
			}
		}
	}

	descriptor_set result = {};
	result.engine = _device.engine;
	result.data.store(set_data);
	return result;
}

} // namespace

descriptor dx_descriptor_impl::dx_descriptor_create(const device& _device, const descriptor_create_params&, size_t _frames_in_flight) {
	descriptor result = {};
	result.engine = _device.engine;
	result.data.store(new dx_descriptor_data());
	result.frames_in_flight = _frames_in_flight;
	return result;
}

descriptor_set dx_descriptor_impl::dx_descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	auto* pipeline_data = _pipeline.data.expect<dx_pipeline_data>();
	return create_set_impl(_descriptor, _device, pipeline_data->root_layout, _params);
}

descriptor_set dx_descriptor_impl::dx_descriptor_set_create_compute(const descriptor& _descriptor, const device& _device, const compute_pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	auto* pipeline_data = _pipeline.data.expect<dx_compute_pipeline_data>();
	return create_set_impl(_descriptor, _device, pipeline_data->root_layout, _params);
}

descriptor_set dx_descriptor_impl::dx_descriptor_set_create_rt(const descriptor& _descriptor, const device& _device, const ray_tracing_pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	auto* pipeline_data = _pipeline.data.expect<dx_rt_pipeline_data>();
	return create_set_impl(_descriptor, _device, pipeline_data->root_layout, _params);
}

void dx_descriptor_impl::dx_descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline&, size_t) {
	auto* set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	for (const auto& [root_index, gpu_address] : set_data->cbv_bindings)
		command_buffer_data->cmd_list->SetGraphicsRootConstantBufferView(static_cast<UINT>(root_index), gpu_address);

	bind_descriptor_tables(set_data, [cmd_list = command_buffer_data->cmd_list.Get()](UINT _root_index, D3D12_GPU_DESCRIPTOR_HANDLE _handle) {
		cmd_list->SetGraphicsRootDescriptorTable(_root_index, _handle);
	});
}

void dx_descriptor_impl::dx_descriptor_set_bind_compute(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const compute_pipeline&, size_t) {
	auto* set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	for (const auto& [root_index, gpu_address] : set_data->cbv_bindings)
		command_buffer_data->cmd_list->SetComputeRootConstantBufferView(static_cast<UINT>(root_index), gpu_address);

	bind_descriptor_tables(set_data, [cmd_list = command_buffer_data->cmd_list.Get()](UINT _root_index, D3D12_GPU_DESCRIPTOR_HANDLE _handle) {
		cmd_list->SetComputeRootDescriptorTable(_root_index, _handle);
	});
}

void dx_descriptor_impl::dx_descriptor_set_bind_rt(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const ray_tracing_pipeline&, size_t) {
	auto* set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto* command_buffer_data = _command_buffer.data.expect<dx_command_buffer_data>();

	for (const auto& [root_index, gpu_address] : set_data->cbv_bindings)
		command_buffer_data->cmd_list->SetComputeRootConstantBufferView(static_cast<UINT>(root_index), gpu_address);

	bind_descriptor_tables(set_data, [cmd_list = command_buffer_data->cmd_list.Get()](UINT _root_index, D3D12_GPU_DESCRIPTOR_HANDLE _handle) {
		cmd_list->SetComputeRootDescriptorTable(_root_index, _handle);
	});
}

void dx_descriptor_impl::dx_descriptor_set_update_cbv(descriptor_set& _descriptor_set, size_t _binding, const buffer& _buffer) {
	auto* set_data = _descriptor_set.data.expect<dx_descriptor_set_data>();
	auto* buffer_data = _buffer.data.expect<dx_buffer_data>();
	if (!set_data->root_layout)
		return;

	const auto* cbv_entry = find_root_entry(*set_data->root_layout, _binding, MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	if (!cbv_entry)
		return;

	const D3D12_GPU_VIRTUAL_ADDRESS gpu_address = buffer_data->resource->GetGPUVirtualAddress();
	for (auto& [root_index, address] : set_data->cbv_bindings) {
		if (root_index != cbv_entry->root_index)
			continue;
		address = gpu_address;
		return;
	}

	set_data->cbv_bindings.push_back({cbv_entry->root_index, gpu_address});
}

void dx_descriptor_impl::dx_descriptor_destroy(descriptor& _descriptor, const device&) {
	auto* descriptor_data = _descriptor.data.expect<dx_descriptor_data>();
	delete descriptor_data;
	_descriptor = {};
}
} // namespace mars::graphics::dx
