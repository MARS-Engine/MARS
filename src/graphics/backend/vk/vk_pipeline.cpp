#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_pipeline.hpp>

#include <algorithm>

namespace mars::graphics::vk {
namespace {
VkPrimitiveTopology vk_topology_from_mars(mars_pipeline_primitive_topology topology) {
	switch (topology) {
	case MARS_PIPELINE_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case MARS_PIPELINE_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case MARS_PIPELINE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	default:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

void set_object_name(vk_device_data* device_data, uint64_t handle, VkObjectType object_type, const std::string& name) {
	if (!device_data->set_debug_name || handle == 0u || name.empty())
		return;

	VkDebugUtilsObjectNameInfoEXT name_info = {};
	name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	name_info.objectType = object_type;
	name_info.objectHandle = handle;
	name_info.pObjectName = name.c_str();
	vk_expect<&vk_device_data::set_debug_name>(device_data, device_data->device, &name_info);
}

vk_sampler_kind combine_sampler_kind(vk_sampler_kind a, vk_sampler_kind b) {
	if (a == vk_sampler_kind::point_clamp || b == vk_sampler_kind::point_clamp)
		return vk_sampler_kind::point_clamp;
	if (a == vk_sampler_kind::linear_clamp || b == vk_sampler_kind::linear_clamp)
		return vk_sampler_kind::linear_clamp;
	return vk_sampler_kind::none;
}

VkDescriptorSetLayout create_explicit_layout(vk_device_data* device_data, const std::vector<pipeline_descriptior_layout>& descriptors, std::vector<vk_pipeline_binding_info>& out_bindings) {
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (const auto& descriptor : descriptors) {
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = static_cast<uint32_t>(descriptor.binding);
		binding.descriptorType = vk_descriptor_type_from_pipeline_descriptor(descriptor.descriptor_type);
		binding.descriptorCount = 1u;
		binding.stageFlags = vk_shader_stage_flags(descriptor.stage);
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

void create_sampler_resources(vk_device_data* device_data, vk_pipeline_data* data, vk_sampler_kind sampler_kind) {
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

pipeline vk_pipeline_impl::vk_pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* shader_data = _setup.pipeline_shader.data.expect<vk_shader_data>();
	auto* render_pass_data = _render_pass.data.expect<vk_render_pass_data>();
	auto* data = new vk_pipeline_data();

	if (shader_data->vertex.module == VK_NULL_HANDLE || shader_data->fragment.module == VK_NULL_HANDLE) {
		mars::logger::error(vk_log_channel(), "Graphics pipeline creation failed: missing shader modules");
		pipeline result;
		result.engine = _device.engine;
		result.data.store(data);
		return result;
	}
	data->debug_name = shader_data->vertex.path + "|" + shader_data->fragment.path;

	data->explicit_set_layout = create_explicit_layout(device_data, _setup.descriptors, data->explicit_bindings);
	if (_setup.push_constant_count > 0u) {
		data->has_push_constants = true;
		data->push_constant_count = _setup.push_constant_count;
		data->push_constant_stage_flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		const uint32_t push_constant_bytes = static_cast<uint32_t>(_setup.push_constant_count * sizeof(uint32_t));
		mars::logger::assert_(push_constant_bytes <= device_data->physical_device_properties.limits.maxPushConstantsSize, vk_log_channel(), "Graphics push constant payload exceeds device limit ({} > {})", push_constant_bytes, device_data->physical_device_properties.limits.maxPushConstantsSize);
	}
	create_sampler_resources(device_data, data, combine_sampler_kind(shader_data->vertex.sampler_kind, shader_data->fragment.sampler_kind));

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

	std::vector<VkVertexInputBindingDescription> binding_descriptions;
	binding_descriptions.reserve(_setup.bindings.size());
	for (const auto& binding : _setup.bindings)
		binding_descriptions.push_back({
			.binding = static_cast<uint32_t>(binding.binding),
			.stride = static_cast<uint32_t>(binding.stride),
			.inputRate = binding.type == MARS_PIPELINE_INPUT_ADVANCE_TYPE_INSTANCE ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX,
		});

	std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
	attribute_descriptions.reserve(_setup.attributes.size());
	for (const auto& attribute : _setup.attributes)
		attribute_descriptions.push_back({
			.location = static_cast<uint32_t>(attribute.location),
			.binding = static_cast<uint32_t>(attribute.binding),
			.format = vk_format_from_mars(attribute.input_format),
			.offset = static_cast<uint32_t>(attribute.offset),
		});

	VkPipelineShaderStageCreateInfo shader_stages[2] = {};
	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = shader_data->vertex.module;
	shader_stages[0].pName = "VSMain";
	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = shader_data->fragment.module;
	shader_stages[1].pName = "PSMain";

	VkPipelineVertexInputStateCreateInfo vertex_input = {};
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
	vertex_input.pVertexBindingDescriptions = binding_descriptions.data();
	vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
	vertex_input.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = vk_topology_from_mars(_setup.primitive_topology);

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1u;
	viewport_state.scissorCount = 1u;

	VkPipelineRasterizationStateCreateInfo raster_state = {};
	raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_state.polygonMode = VK_POLYGON_MODE_FILL;
	raster_state.cullMode = VK_CULL_MODE_NONE;
	raster_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	raster_state.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo multisample_state = {};
	multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (_setup.has_alpha_blend_override && _setup.alpha_blend_enable) {
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	VkPipelineColorBlendStateCreateInfo color_blend_state = {};
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.attachmentCount = 1u;
	color_blend_state.pAttachments = &color_blend_attachment;

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = render_pass_data->depth_format != MARS_DEPTH_FORMAT_UNDEFINED ? VK_TRUE : VK_FALSE;
	depth_stencil.depthWriteEnable = render_pass_data->depth_format != MARS_DEPTH_FORMAT_UNDEFINED ? VK_TRUE : VK_FALSE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;

	if (_setup.has_depth_test_override)
		depth_stencil.depthTestEnable = _setup.depth_test_enable ? VK_TRUE : VK_FALSE;
	if (_setup.has_depth_write_override)
		depth_stencil.depthWriteEnable = _setup.depth_write_enable ? VK_TRUE : VK_FALSE;
	if (_setup.has_depth_compare_override)
		depth_stencil.depthCompareOp = vk_compare_op_from_mars(_setup.depth_compare);

	VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2u;
	dynamic_state.pDynamicStates = dynamic_states;

	const VkFormat color_format = render_pass_data->actual_color_format != VK_FORMAT_UNDEFINED ? render_pass_data->actual_color_format : vk_format_from_mars(render_pass_data->format);
	const VkFormat depth_format = render_pass_data->depth_format != MARS_DEPTH_FORMAT_UNDEFINED ? vk_depth_format_from_mars(render_pass_data->depth_format) : VK_FORMAT_UNDEFINED;

	VkPipelineRenderingCreateInfo rendering_info = {};
	rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	rendering_info.colorAttachmentCount = 1u;
	rendering_info.pColorAttachmentFormats = &color_format;
	rendering_info.depthAttachmentFormat = depth_format;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = &rendering_info;
	pipeline_info.stageCount = 2u;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &raster_state;
	pipeline_info.pMultisampleState = &multisample_state;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blend_state;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = data->layout;
	vk_expect<vkCreateGraphicsPipelines>(device_data->device, VK_NULL_HANDLE, 1u, &pipeline_info, nullptr, &data->pipeline);
	if (data->pipeline != VK_NULL_HANDLE) {
		const std::string pipeline_name =
			"gfx|" + shader_data->vertex.path + "|" + shader_data->fragment.path +
			"|depth=" + std::to_string(static_cast<int>(depth_format));
		set_object_name(device_data, reinterpret_cast<uint64_t>(data->pipeline), VK_OBJECT_TYPE_PIPELINE, pipeline_name);
	}

	pipeline result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void vk_pipeline_impl::vk_pipeline_bind(const pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params) {
	auto* pipeline_data = _pipeline.data.expect<vk_pipeline_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* device_data = command_buffer_data->device_data;

	vkCmdBindPipeline(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_data->pipeline);

	VkViewport viewport = {};
	viewport.y = static_cast<float>(_params.size.y);
	viewport.width = static_cast<float>(_params.size.x);
	viewport.height = -static_cast<float>(_params.size.y);
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(_params.size.x), static_cast<uint32_t>(_params.size.y)}};
	vkCmdSetViewport(command_buffer_data->command_buffer, 0u, 1u, &viewport);
	vkCmdSetScissor(command_buffer_data->command_buffer, 0u, 1u, &scissor);

	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_data->layout, 0u, 1u, &device_data->bindless_set, 0u, nullptr);
	if (pipeline_data->explicit_set_layout == VK_NULL_HANDLE)
		vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_data->layout, 1u, 1u, &device_data->empty_set, 0u, nullptr);
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_data->layout, 2u, 1u, &device_data->empty_set, 0u, nullptr);

	const VkDescriptorSet sampler_set = pipeline_data->sampler_set_layout != VK_NULL_HANDLE ? pipeline_data->sampler_set : device_data->empty_set;
	vkCmdBindDescriptorSets(command_buffer_data->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_data->layout, 3u, 1u, &sampler_set, 0u, nullptr);
}

void vk_pipeline_impl::vk_pipeline_destroy(pipeline& _pipeline, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _pipeline.data.expect<vk_pipeline_data>();

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
