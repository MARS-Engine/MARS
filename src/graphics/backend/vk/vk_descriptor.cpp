#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_descriptor.hpp>

#include <algorithm>

namespace mars::graphics::vk {
namespace {
VkDescriptorType descriptor_pool_type_from_mars(mars_descriptor_type descriptor_type) {
	switch (descriptor_type) {
	case MARS_DESCRIPTOR_TYPE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case MARS_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case MARS_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case MARS_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case MARS_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	case MARS_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
	default:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	}
}

template <typename PipelineDataT>
descriptor_set create_set_impl(
	const descriptor& _descriptor,
	const device& _device,
	PipelineDataT* pipeline_data,
	const std::vector<descriptor_set_create_params>& _params
) {
	auto* descriptor_data = _descriptor.data.expect<vk_descriptor_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* set_data = new vk_descriptor_set_data();
	set_data->device_data = device_data;
	set_data->bindings = pipeline_data ? pipeline_data->explicit_bindings : std::vector<vk_pipeline_binding_info>{};

	descriptor_set result;
	result.engine = _device.engine;
	result.data.store(set_data);

	if (!pipeline_data || pipeline_data->explicit_set_layout == VK_NULL_HANDLE || set_data->bindings.empty())
		return result;

	const size_t set_count = _params.empty() ? descriptor_data->frames_in_flight : _params.size();
	set_data->sets.resize(set_count);

	std::vector<VkDescriptorSetLayout> layouts(set_count, pipeline_data->explicit_set_layout);
	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = descriptor_data->pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(set_count);
	alloc_info.pSetLayouts = layouts.data();
	vk_expect<vkAllocateDescriptorSets>(device_data->device, &alloc_info, set_data->sets.data());

	for (size_t set_index = 0u; set_index < set_count; ++set_index) {
		const auto& params = _params.empty() ? descriptor_set_create_params{} : _params[set_index];
		std::vector<VkDescriptorBufferInfo> buffer_infos;
		std::vector<VkWriteDescriptorSet> writes;

		for (const auto& [buffer_handle, binding] : params.buffers) {
			const auto binding_info = vk_find_binding_info(set_data->bindings, static_cast<uint32_t>(binding));
			if (!binding_info.has_value() || binding_info->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				continue;

			buffer_infos.push_back({
				.buffer = buffer_handle.data.expect<vk_buffer_data>()->buffer,
				.offset = 0u,
				.range = buffer_handle.data.expect<vk_buffer_data>()->size,
			});

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = set_data->sets[set_index];
			write.dstBinding = static_cast<uint32_t>(binding);
			write.descriptorCount = 1u;
			write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write.pBufferInfo = &buffer_infos.back();
			writes.push_back(write);
		}

		if (!writes.empty())
			vkUpdateDescriptorSets(device_data->device, static_cast<uint32_t>(writes.size()), writes.data(), 0u, nullptr);
	}

	return result;
}
} // namespace

descriptor vk_descriptor_impl::vk_descriptor_create(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_descriptor_data();
	data->frames_in_flight = _frames_in_flight;

	std::vector<VkDescriptorPoolSize> pool_sizes;
	for (size_t descriptor_type = 0u; descriptor_type < _params.descriptors_size.size(); ++descriptor_type) {
		const size_t count = _params.descriptors_size[descriptor_type];
		if (count == 0u)
			continue;
		pool_sizes.push_back({
			.type = descriptor_pool_type_from_mars(static_cast<mars_descriptor_type>(descriptor_type)),
			.descriptorCount = static_cast<uint32_t>(count),
		});
	}
	if (pool_sizes.empty())
		pool_sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1u});

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = static_cast<uint32_t>((std::max)(_params.max_sets, size_t{1}));
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
	vk_expect<vkCreateDescriptorPool>(device_data->device, &pool_info, nullptr, &data->pool);

	descriptor result;
	result.engine = _device.engine;
	result.data.store(data);
	result.frames_in_flight = _frames_in_flight;
	return result;
}

descriptor_set vk_descriptor_impl::vk_descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	return create_set_impl(_descriptor, _device, _pipeline.data.expect<vk_pipeline_data>(), _params);
}

descriptor_set vk_descriptor_impl::vk_descriptor_set_create_compute(const descriptor& _descriptor, const device& _device, const compute_pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
	return create_set_impl(_descriptor, _device, _pipeline.data.expect<vk_compute_pipeline_data>(), _params);
}

void vk_descriptor_impl::vk_descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame) {
	auto* set_data = _descriptor_set.data.expect<vk_descriptor_set_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* pipeline_data = _pipeline.data.expect<vk_pipeline_data>();
	if (set_data->sets.empty() || pipeline_data->explicit_set_layout == VK_NULL_HANDLE)
		return;

	const VkDescriptorSet set = set_data->sets[_current_frame % set_data->sets.size()];
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_data->layout, 1u, 1u, &set, 0u, nullptr);
}

void vk_descriptor_impl::vk_descriptor_set_bind_compute(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const compute_pipeline& _pipeline, size_t _current_frame) {
	auto* set_data = _descriptor_set.data.expect<vk_descriptor_set_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* pipeline_data = _pipeline.data.expect<vk_compute_pipeline_data>();
	if (set_data->sets.empty() || pipeline_data->explicit_set_layout == VK_NULL_HANDLE)
		return;

	const VkDescriptorSet set = set_data->sets[_current_frame % set_data->sets.size()];
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_data->layout, 1u, 1u, &set, 0u, nullptr);
}

void vk_descriptor_impl::vk_descriptor_set_update_cbv(descriptor_set& _descriptor_set, size_t _binding, const buffer& _buffer) {
	auto* set_data = _descriptor_set.data.expect<vk_descriptor_set_data>();
	auto* buffer_data = _buffer.data.expect<vk_buffer_data>();
	const auto binding_info = vk_find_binding_info(set_data->bindings, static_cast<uint32_t>(_binding));
	if (!binding_info.has_value() || binding_info->type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || set_data->sets.empty())
		return;

	for (VkDescriptorSet set : set_data->sets) {
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.buffer = buffer_data->buffer;
		buffer_info.offset = 0u;
		buffer_info.range = buffer_data->size;

		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = set;
		write.dstBinding = static_cast<uint32_t>(_binding);
		write.descriptorCount = 1u;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &buffer_info;
		vkUpdateDescriptorSets(set_data->device_data->device, 1u, &write, 0u, nullptr);
	}
}

void vk_descriptor_impl::vk_descriptor_destroy(descriptor& _descriptor, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _descriptor.data.expect<vk_descriptor_data>();
	if (data->pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(device_data->device, data->pool, nullptr);
	delete data;
	_descriptor = {};
}
} // namespace mars::graphics::vk
