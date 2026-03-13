#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_compute_pipeline.hpp>

#include <algorithm>

namespace mars::graphics::vk {
namespace {
VkDescriptorSetLayout create_explicit_layout(vk_device_data* device_data, const std::vector<pipeline_descriptior_layout>& descriptors, std::vector<vk_pipeline_binding_info>& out_bindings) {
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (const auto& descriptor : descriptors) {
		if (descriptor.descriptor_type != MARS_PIPELINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			continue;

		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = static_cast<uint32_t>(descriptor.binding);
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.descriptorCount = 1u;
		binding.stageFlags = VK_SHADER_STAGE_ALL;
		bindings.push_back(binding);
		out_bindings.push_back({binding.binding, binding.descriptorType});
	}
	if (bindings.empty())
		return VK_NULL_HANDLE;

	std::sort(bindings.begin(), bindings.end(), [](const auto& lhs, const auto& rhs) { return lhs.binding < rhs.binding; });
	std::sort(out_bindings.begin(), out_bindings.end(), [](const auto& lhs, const auto& rhs) { return lhs.binding < rhs.binding; });

	VkDescriptorSetLayoutCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = static_cast<uint32_t>(bindings.size());
	create_info.pBindings = bindings.data();

	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	vk_expect<vkCreateDescriptorSetLayout>(device_data->device, &create_info, nullptr, &layout);
	return layout;
}

void create_sampler_resources(vk_device_data* device_data, vk_compute_pipeline_data* data, vk_sampler_kind sampler_kind) {
	if (sampler_kind == vk_sampler_kind::none)
		return;

	VkDescriptorSetLayoutBinding binding = {};
	binding.binding = 0u;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	binding.descriptorCount = 1u;
	binding.stageFlags = VK_SHADER_STAGE_ALL;

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1u;
	layout_info.pBindings = &binding;
	vk_expect<vkCreateDescriptorSetLayout>(device_data->device, &layout_info, nullptr, &data->sampler_set_layout);

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = device_data->sampler_descriptor_pool;
	alloc_info.descriptorSetCount = 1u;
	alloc_info.pSetLayouts = &data->sampler_set_layout;
	vk_expect<vkAllocateDescriptorSets>(device_data->device, &alloc_info, &data->sampler_set);

	VkDescriptorImageInfo image_info = {};
	image_info.sampler = vk_get_sampler(device_data, sampler_kind);

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = data->sampler_set;
	write.dstBinding = 0u;
	write.descriptorCount = 1u;
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	write.pImageInfo = &image_info;
	vkUpdateDescriptorSets(device_data->device, 1u, &write, 0u, nullptr);

	data->uses_sampler = true;
	data->sampler_kind = sampler_kind;
}
} // namespace

compute_pipeline vk_compute_pipeline_impl::vk_compute_pipeline_create(const device& _device, const compute_pipeline_setup& _setup) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* shader_data = _setup.pipeline_shader.data.expect<vk_shader_data>();
	auto* data = new vk_compute_pipeline_data();

	if (shader_data->compute.module == VK_NULL_HANDLE) {
		mars::logger::error(vk_log_channel(), "Compute pipeline creation failed: missing compute shader module");
		compute_pipeline result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}

	data->explicit_set_layout = create_explicit_layout(device_data, _setup.descriptors, data->explicit_bindings);
	if (_setup.push_constant_count > 0u) {
		data->has_push_constants = true;
		data->push_constant_count = _setup.push_constant_count;
		data->push_constant_stage_flags = VK_SHADER_STAGE_COMPUTE_BIT;
		const uint32_t push_constant_bytes = static_cast<uint32_t>(_setup.push_constant_count * sizeof(uint32_t));
		mars::logger::assert_(
			push_constant_bytes <= device_data->physical_device_properties.limits.maxPushConstantsSize,
			vk_log_channel(),
			"Compute push constant payload exceeds device limit ({} > {})",
			push_constant_bytes,
			device_data->physical_device_properties.limits.maxPushConstantsSize
		);
	}
	create_sampler_resources(device_data, data, shader_data->compute.sampler_kind);

	VkDescriptorSetLayout set_layouts[] = {
		device_data->bindless_set_layout,
		data->explicit_set_layout != VK_NULL_HANDLE ? data->explicit_set_layout : device_data->empty_set_layout,
		device_data->empty_set_layout,
		data->sampler_set_layout != VK_NULL_HANDLE ? data->sampler_set_layout : device_data->empty_set_layout,
	};

	VkPushConstantRange push_constant_range = {};
	push_constant_range.stageFlags = data->push_constant_stage_flags;
	push_constant_range.offset = 0u;
	push_constant_range.size = static_cast<uint32_t>(data->push_constant_count * sizeof(uint32_t));

	VkPipelineLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.setLayoutCount = 4u;
	layout_info.pSetLayouts = set_layouts;
	if (data->has_push_constants) {
		layout_info.pushConstantRangeCount = 1u;
		layout_info.pPushConstantRanges = &push_constant_range;
	}
	vk_expect<vkCreatePipelineLayout>(device_data->device, &layout_info, nullptr, &data->layout);

	VkPipelineShaderStageCreateInfo shader_stage = {};
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage.module = shader_data->compute.module;
	shader_stage.pName = "CSMain";

	VkComputePipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.stage = shader_stage;
	pipeline_info.layout = data->layout;
	vk_expect<vkCreateComputePipelines>(device_data->device, VK_NULL_HANDLE, 1u, &pipeline_info, nullptr, &data->pipeline);

	compute_pipeline result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void vk_compute_pipeline_impl::vk_compute_pipeline_bind(const compute_pipeline& _pipeline, const command_buffer& _command_buffer) {
	auto* pipeline_data = _pipeline.data.expect<vk_compute_pipeline_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* device_data = command_buffer_data->device_data;

	vkCmdBindPipeline(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_data->pipeline);
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_data->layout, 0u, 1u, &device_data->bindless_set, 0u, nullptr);
	if (pipeline_data->explicit_set_layout == VK_NULL_HANDLE)
		vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_data->layout, 1u, 1u, &device_data->empty_set, 0u, nullptr);
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_data->layout, 2u, 1u, &device_data->empty_set, 0u, nullptr);

	const VkDescriptorSet sampler_set = pipeline_data->sampler_set_layout != VK_NULL_HANDLE ? pipeline_data->sampler_set : device_data->empty_set;
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_data->layout, 3u, 1u, &sampler_set, 0u, nullptr);
}

void vk_compute_pipeline_impl::vk_compute_pipeline_destroy(compute_pipeline& _pipeline, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _pipeline.data.expect<vk_compute_pipeline_data>();

	if (data->sampler_set != VK_NULL_HANDLE)
		vk_expect<vkFreeDescriptorSets>(device_data->device, device_data->sampler_descriptor_pool, 1u, &data->sampler_set);
	if (data->sampler_set_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(device_data->device, data->sampler_set_layout, nullptr);
	if (data->explicit_set_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(device_data->device, data->explicit_set_layout, nullptr);
	if (data->pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(device_data->device, data->pipeline, nullptr);
	if (data->layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(device_data->device, data->layout, nullptr);

	delete data;
	_pipeline = {};
}
} // namespace mars::graphics::vk
