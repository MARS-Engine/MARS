#include <mars/graphics/backend/vk/vk_ray_tracing_pipeline.hpp>

#include "vk_internal.hpp"

#include <cstring>
#include <vector>

namespace mars::graphics::vk {
ray_tracing_pipeline vk_ray_tracing_pipeline_impl::vk_ray_tracing_pipeline_create(const device& _device, const ray_tracing_pipeline_setup& _setup) {
	auto* device_data = _device.data.expect<vk_device_data>();

	std::vector<VkPipelineShaderStageCreateInfo> stages;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

	auto push_stage = [&](VkShaderModule _module, VkShaderStageFlagBits _stage_flag, const char* _entry_name) -> uint32_t {
		VkPipelineShaderStageCreateInfo stage_info = {};
		stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_info.stage = _stage_flag;
		stage_info.module = _module;
		stage_info.pName = _entry_name;
		stages.push_back(stage_info);
		return static_cast<uint32_t>(stages.size()) - 1u;
	};

	{
		auto* shader_data = _setup.raygen_shader.data.expect<vk_shader_data>();
		const uint32_t stage_index = push_stage(shader_data->compute.module, VK_SHADER_STAGE_RAYGEN_BIT_KHR, "RayGen");
		VkRayTracingShaderGroupCreateInfoKHR group = {};
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = stage_index;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		groups.push_back(group);
	}

	for (const auto& miss_shader : _setup.miss_shaders) {
		auto* shader_data = miss_shader.data.expect<vk_shader_data>();
		const uint32_t stage_index = push_stage(shader_data->compute.module, VK_SHADER_STAGE_MISS_BIT_KHR, "Miss");
		VkRayTracingShaderGroupCreateInfoKHR group = {};
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
		group.generalShader = stage_index;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		groups.push_back(group);
	}

	for (const auto& hit_group : _setup.hit_groups) {
		auto* shader_data = hit_group.closest_hit_shader.data.expect<vk_shader_data>();
		const uint32_t closest_hit_index = push_stage(shader_data->compute.module, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, "ClosestHit");
		VkRayTracingShaderGroupCreateInfoKHR group = {};
		group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = closest_hit_index;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
		groups.push_back(group);
	}

	auto* rt_data = new vk_rt_pipeline_data();
	rt_data->raygen_group_count = 1u;
	rt_data->miss_group_count = static_cast<uint32_t>(_setup.miss_shaders.size());
	rt_data->hit_group_count = static_cast<uint32_t>(_setup.hit_groups.size());

	std::vector<VkDescriptorSetLayout> set_layouts;
	set_layouts.push_back(device_data->bindless_set_layout);

	if (!_setup.descriptors.empty()) {
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		for (const auto& descriptor : _setup.descriptors) {
			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = static_cast<uint32_t>(descriptor.binding);
			binding.descriptorType = vk_descriptor_type_from_pipeline_descriptor(descriptor.descriptor_type);
			binding.descriptorCount = 1u;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			bindings.push_back(binding);
			rt_data->explicit_bindings.push_back({
				.binding = binding.binding,
				.register_space = static_cast<uint32_t>(descriptor.register_space),
				.type = binding.descriptorType,
			});
		}

		VkDescriptorSetLayoutCreateInfo layout_ci = {};
		layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_ci.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_ci.pBindings = bindings.data();
		vk_expect<vkCreateDescriptorSetLayout>(device_data->device, &layout_ci, nullptr, &rt_data->explicit_set_layout);
		set_layouts.push_back(rt_data->explicit_set_layout);
	}

	VkPipelineLayoutCreateInfo layout_ci = {};
	layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_ci.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
	layout_ci.pSetLayouts = set_layouts.data();
	vk_expect<vkCreatePipelineLayout>(device_data->device, &layout_ci, nullptr, &rt_data->layout);

	VkRayTracingPipelineCreateInfoKHR rt_ci = {};
	rt_ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rt_ci.stageCount = static_cast<uint32_t>(stages.size());
	rt_ci.pStages = stages.data();
	rt_ci.groupCount = static_cast<uint32_t>(groups.size());
	rt_ci.pGroups = groups.data();
	rt_ci.maxPipelineRayRecursionDepth = _setup.max_recursion_depth;
	rt_ci.layout = rt_data->layout;

	const VkResult create_result = vk_expect<&vk_device_data::create_ray_tracing_pipelines>(device_data, device_data->device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1u, &rt_ci, nullptr, &rt_data->pipeline);
	if (create_result != VK_SUCCESS && create_result != VK_SUBOPTIMAL_KHR) {
		if (rt_data->layout != VK_NULL_HANDLE)
			vkDestroyPipelineLayout(device_data->device, rt_data->layout, nullptr);
		if (rt_data->explicit_set_layout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(device_data->device, rt_data->explicit_set_layout, nullptr);
		delete rt_data;
		return {};
	}

	ray_tracing_pipeline result = {};
	result.engine = _device.engine;
	result.data.store(rt_data);
	return result;
}

void vk_ray_tracing_pipeline_impl::vk_ray_tracing_pipeline_bind(const ray_tracing_pipeline& _pipeline, const command_buffer& _command_buffer) {
	auto* rt_data = _pipeline.data.expect<vk_rt_pipeline_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();

	vkCmdBindPipeline(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_data->pipeline);
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rt_data->layout, 0u, 1u, &command_buffer_data->device_data->bindless_set, 0u, nullptr);
}

void vk_ray_tracing_pipeline_impl::vk_ray_tracing_pipeline_destroy(ray_tracing_pipeline& _pipeline, const device& _device) {
	auto* rt_data = _pipeline.data.expect<vk_rt_pipeline_data>();
	auto* device_data = _device.data.expect<vk_device_data>();

	vkDestroyPipeline(device_data->device, rt_data->pipeline, nullptr);
	vkDestroyPipelineLayout(device_data->device, rt_data->layout, nullptr);
	if (rt_data->explicit_set_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(device_data->device, rt_data->explicit_set_layout, nullptr);
	delete rt_data;
}

void vk_ray_tracing_pipeline_impl::vk_rt_pipeline_write_shader_identifiers(const ray_tracing_pipeline& _pipeline, const device& _device, uint32_t _first_group, uint32_t _count, void* _dst, uint32_t _dst_stride) {
	auto* rt_data = _pipeline.data.expect<vk_rt_pipeline_data>();
	auto* device_data = _device.data.expect<vk_device_data>();

	const uint32_t handle_size = device_data->rt_pipeline_properties.shaderGroupHandleSize;
	const uint32_t total = rt_data->raygen_group_count + rt_data->miss_group_count + rt_data->hit_group_count;

	std::vector<uint8_t> handles(total * handle_size);
	device_data->get_ray_tracing_shader_group_handles(device_data->device, rt_data->pipeline, 0u, total, handles.size(), handles.data());

	auto* dst = static_cast<uint8_t*>(_dst);
	for (uint32_t i = 0u; i < _count; ++i)
		std::memcpy(dst + i * _dst_stride, handles.data() + (_first_group + i) * handle_size, handle_size);
}

} // namespace mars::graphics::vk
