#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_buffer.hpp>
#include <mars/graphics/backend/vk/vk_indirect_executor.hpp>

#include <algorithm>
#include <array>

namespace mars::graphics::vk {
namespace {
constexpr uint32_t k_graphics_indirect_draw_arg_sentinel = 0x80000000u;

void destroy_preprocess_buffer(vk_device_data* device_data, vk_indirect_executor_data::preprocess_buffer_entry& entry) {
	if (!device_data)
		return;
	vk_destroy_buffer(device_data, entry.buffer, entry.memory);
	entry = {};
}

bool create_preprocess_buffer(vk_device_data* device_data, vk_indirect_executor_data::preprocess_buffer_entry& entry, VkDeviceSize size, uint32_t max_sequence_count) {
	if (size == 0u) {
		entry.size = 0u;
		entry.max_sequence_count = max_sequence_count;
		entry.device_address = 0u;
		return true;
	}

	VkBufferUsageFlags2CreateInfo usage_2 = {};
	usage_2.sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO;
	usage_2.usage = VK_BUFFER_USAGE_2_PREPROCESS_BUFFER_BIT_EXT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;

	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.pNext = &usage_2;
	create_info.size = size;
	create_info.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vk_expect<vkCreateBuffer>(device_data->device, &create_info, nullptr, &entry.buffer) != VK_SUCCESS)
		return false;

	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements(device_data->device, entry.buffer, &requirements);

	VkMemoryAllocateFlagsInfo allocate_flags = {};
	allocate_flags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.pNext = &allocate_flags;
	allocate_info.allocationSize = requirements.size;
	allocate_info.memoryTypeIndex = vk_find_memory_type(device_data, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vk_expect<vkAllocateMemory>(device_data->device, &allocate_info, nullptr, &entry.memory) != VK_SUCCESS) {
		destroy_preprocess_buffer(device_data, entry);
		return false;
	}

	if (vk_expect<vkBindBufferMemory>(device_data->device, entry.buffer, entry.memory, 0u) != VK_SUCCESS) {
		destroy_preprocess_buffer(device_data, entry);
		return false;
	}

	VkBufferDeviceAddressInfo address_info = {};
	address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	address_info.buffer = entry.buffer;
	entry.device_address = vkGetBufferDeviceAddress(device_data->device, &address_info);
	entry.size = requirements.size;
	entry.max_sequence_count = max_sequence_count;
	return true;
}

vk_indirect_executor_data::preprocess_buffer_entry* get_preprocess_buffer_entry(vk_indirect_executor_data* executor_data, vk_command_buffer_data* command_buffer_data, uint32_t max_sequence_count, uint32_t max_draw_count) {
	auto* device_data = executor_data->device_data;
	VkGeneratedCommandsPipelineInfoEXT pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT;
	pipeline_info.pipeline = executor_data->pipeline;

	VkGeneratedCommandsMemoryRequirementsInfoEXT memory_info = {};
	memory_info.sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_EXT;
	memory_info.pNext = &pipeline_info;
	memory_info.indirectExecutionSet = VK_NULL_HANDLE;
	memory_info.indirectCommandsLayout = executor_data->indirect_layout;
	memory_info.maxSequenceCount = max_sequence_count;
	memory_info.maxDrawCount = max_draw_count;

	VkMemoryRequirements2 requirements = {};
	requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	device_data->get_generated_commands_memory_requirements(device_data->device, &memory_info, &requirements);

	auto entry_it = std::find_if(executor_data->preprocess_buffers.begin(), executor_data->preprocess_buffers.end(), [command_buffer_data](const auto& entry) { return entry.command_buffer == command_buffer_data; });

	if (entry_it == executor_data->preprocess_buffers.end()) {
		executor_data->preprocess_buffers.push_back({});
		entry_it = executor_data->preprocess_buffers.end() - 1;
		entry_it->command_buffer = command_buffer_data;
	}

	if (entry_it->buffer == VK_NULL_HANDLE ||
		entry_it->size < requirements.memoryRequirements.size ||
		entry_it->max_sequence_count < max_sequence_count) {
		destroy_preprocess_buffer(device_data, *entry_it);
		entry_it->command_buffer = command_buffer_data;
		if (!create_preprocess_buffer(device_data, *entry_it, requirements.memoryRequirements.size, max_sequence_count)) {
			mars::logger::error(vk_log_channel(), "Failed to allocate preprocess buffer for generated commands");
			return nullptr;
		}
	}

	return &(*entry_it);
}

VkIndirectCommandsLayoutEXT create_indirect_layout(vk_device_data* device_data, VkPipelineLayout pipeline_layout, VkShaderStageFlags shader_stages, uint32_t constant_bytes, uint32_t command_offset, uint32_t command_stride, VkIndirectCommandsTokenTypeEXT command_token, bool use_sequence_index = false) {
	VkPushConstantRange push_constant_range = {};
	push_constant_range.stageFlags = shader_stages;
	push_constant_range.offset = 0u;
	push_constant_range.size = constant_bytes;

	VkIndirectCommandsPushConstantTokenEXT push_constant_token = {};
	push_constant_token.updateRange = push_constant_range;

	std::array<VkIndirectCommandsLayoutTokenEXT, 2> tokens = {};
	tokens[0].sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT;
	tokens[0].type = use_sequence_index ? VK_INDIRECT_COMMANDS_TOKEN_TYPE_SEQUENCE_INDEX_EXT : VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_EXT;
	tokens[0].data.pPushConstant = &push_constant_token;
	tokens[0].offset = 0u;

	tokens[1].sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_EXT;
	tokens[1].type = command_token;
	tokens[1].offset = command_offset;

	VkIndirectCommandsLayoutCreateInfoEXT create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_EXT;
	create_info.shaderStages = shader_stages;
	create_info.indirectStride = command_stride;
	create_info.pipelineLayout = pipeline_layout;
	create_info.tokenCount = static_cast<uint32_t>(tokens.size());
	create_info.pTokens = tokens.data();

	VkIndirectCommandsLayoutEXT layout = VK_NULL_HANDLE;
	device_data->create_indirect_commands_layout(device_data->device, &create_info, nullptr, &layout);
	return layout;
}

indirect_executor make_executor(const device& dev, mars_command_signature_type type, bool with_constants, VkPipelineBindPoint bind_point, VkPipeline pipeline, VkPipelineLayout pipeline_layout, VkShaderStageFlags shader_stages, uint32_t embedded_constant_bytes, uint32_t command_offset, uint32_t command_stride, VkIndirectCommandsTokenTypeEXT command_token, bool use_sequence_index = false) {
	auto* device_data = dev.data.expect<vk_device_data>();
	auto* data = new vk_indirect_executor_data();
	data->device_data = device_data;
	data->type = type;
	data->has_embedded_constants = with_constants;
	data->bind_point = bind_point;
	data->pipeline = pipeline;
	data->pipeline_layout = pipeline_layout;
	data->shader_stages = shader_stages;
	data->embedded_constant_bytes = embedded_constant_bytes;
	data->command_offset = command_offset;
	data->command_stride = command_stride;

	if (with_constants && device_data->supports_device_generated_commands)
		data->indirect_layout = create_indirect_layout(device_data, pipeline_layout, shader_stages, embedded_constant_bytes, command_offset, command_stride, command_token, use_sequence_index);

	indirect_executor result;
	result.engine = dev.engine;
	result.data.store(data);
	return result;
}

void record_embedded_impl(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	auto* ex_data = ex.data.expect<vk_indirect_executor_data>();
	auto* cmd_data = cmd.data.expect<vk_command_buffer_data>();
	auto* arg_data = arg_buf.data.expect<vk_buffer_data>();

	if (!ex_data->device_data->supports_device_generated_commands || ex_data->indirect_layout == VK_NULL_HANDLE) {
		return;
	}

	const uint32_t stride_limited_count = ex_data->command_stride == 0u ? max_count : static_cast<uint32_t>(std::min(arg_data->size / ex_data->command_stride, static_cast<size_t>(max_count)));
	const uint32_t sequence_limit = std::min(stride_limited_count, ex_data->device_data->device_generated_commands_properties.maxIndirectSequenceCount);
	if (sequence_limit == 0u)
		return;

	const uint32_t max_draw_count = sequence_limit;

	vk_buffer_impl::vk_buffer_transition(cmd, arg_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);

	constexpr bool ignore_sequence_count = false;
	VkDeviceAddress sequence_count_address = 0u;
	if (count_buf && !ignore_sequence_count) {
		vk_buffer_impl::vk_buffer_transition(cmd, *count_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);
		sequence_count_address = count_buf->data.expect<vk_buffer_data>()->device_address;
	}

	auto* preprocess_entry = get_preprocess_buffer_entry(ex_data, cmd_data, sequence_limit, max_draw_count);
	if (!preprocess_entry)
		return;

	VkGeneratedCommandsPipelineInfoEXT pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_PIPELINE_INFO_EXT;
	pipeline_info.pipeline = ex_data->pipeline;

	VkGeneratedCommandsInfoEXT generated_info = {};
	generated_info.sType = VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_EXT;
	generated_info.pNext = &pipeline_info;
	generated_info.shaderStages = ex_data->shader_stages;
	generated_info.indirectExecutionSet = VK_NULL_HANDLE;
	generated_info.indirectCommandsLayout = ex_data->indirect_layout;
	generated_info.indirectAddress = arg_data->device_address;
	generated_info.indirectAddressSize = static_cast<VkDeviceSize>(std::min(arg_data->size, static_cast<size_t>(sequence_limit) * static_cast<size_t>(ex_data->command_stride)));
	generated_info.preprocessAddress = preprocess_entry->device_address;
	generated_info.preprocessSize = preprocess_entry->size;
	generated_info.maxSequenceCount = sequence_limit;
	generated_info.sequenceCountAddress = sequence_count_address;
	generated_info.maxDrawCount = max_draw_count;

	ex_data->device_data->cmd_execute_generated_commands(cmd_data->command_buffer, VK_FALSE, &generated_info);
}

void record_graphics_embedded_indirect(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	auto* ex_data = ex.data.expect<vk_indirect_executor_data>();
	auto* cmd_data = cmd.data.expect<vk_command_buffer_data>();
	auto* arg_data = arg_buf.data.expect<vk_buffer_data>();
	mars::logger::assert_(arg_data->srv_bindless_idx != std::numeric_limits<uint32_t>::max(), vk_log_channel(), "graphics embedded indirect requires an SRV-capable argument buffer");

	vk_buffer_impl::vk_buffer_transition(cmd, arg_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);

	VkBuffer count_buffer_handle = VK_NULL_HANDLE;
	if (count_buf) {
		vk_buffer_impl::vk_buffer_transition(cmd, *count_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);
		count_buffer_handle = count_buf->data.expect<vk_buffer_data>()->buffer;
	}

	const uint32_t draw_arg_slot = arg_data->srv_bindless_idx | k_graphics_indirect_draw_arg_sentinel;
	const uint32_t draw_count = max_count;
	vkCmdPushConstants(cmd_data->command_buffer, ex_data->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &draw_arg_slot);

	if (count_buffer_handle != VK_NULL_HANDLE && draw_count == max_count)
		vkCmdDrawIndexedIndirectCount(cmd_data->command_buffer, arg_data->buffer, ex_data->command_offset, count_buffer_handle, 0u, max_count, ex_data->command_stride);
	else
		vkCmdDrawIndexedIndirect(cmd_data->command_buffer, arg_data->buffer, ex_data->command_offset, draw_count, ex_data->command_stride);
}

void record_impl(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	auto* ex_data = ex.data.expect<vk_indirect_executor_data>();
	auto* cmd_data = cmd.data.expect<vk_command_buffer_data>();
	auto* arg_data = arg_buf.data.expect<vk_buffer_data>();

	if (ex_data->has_embedded_constants) {
		if (ex_data->type == MARS_COMMAND_SIGNATURE_DRAW_INDEXED &&
			(!ex_data->device_data->supports_device_generated_commands || !arg_data->host_visible || count_buf != nullptr)) {
			record_graphics_embedded_indirect(ex, cmd, arg_buf, max_count, count_buf);
			return;
		}
		if (!ex_data->device_data->supports_device_generated_commands) {
			return;
		}
		record_embedded_impl(ex, cmd, arg_buf, max_count, count_buf);
		return;
	}

	vk_buffer_impl::vk_buffer_transition(cmd, arg_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);

	VkBuffer count_buffer_handle = VK_NULL_HANDLE;

	if (count_buf) {
		vk_buffer_impl::vk_buffer_transition(cmd, *count_buf, MARS_BUFFER_STATE_INDIRECT_ARGUMENT);
		count_buffer_handle = count_buf->data.expect<vk_buffer_data>()->buffer;
	}

	if (ex_data->type == MARS_COMMAND_SIGNATURE_DISPATCH) {
		vkCmdDispatchIndirect(cmd_data->command_buffer, arg_data->buffer, 0u);
		return;
	}

	if (count_buffer_handle != VK_NULL_HANDLE)
		vkCmdDrawIndexedIndirectCount(cmd_data->command_buffer, arg_data->buffer, 0u, count_buffer_handle, 0u, max_count, sizeof(VkDrawIndexedIndirectCommand));
	else
		vkCmdDrawIndexedIndirect(cmd_data->command_buffer, arg_data->buffer, 0u, max_count, sizeof(VkDrawIndexedIndirectCommand));
}
} // namespace

indirect_executor vk_indirect_executor_impl::create(const device& dev, mars_command_signature_type type) {
	auto pipeline_type = type == MARS_COMMAND_SIGNATURE_DISPATCH ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
	auto shader_type = type == MARS_COMMAND_SIGNATURE_DISPATCH ? VK_SHADER_STAGE_COMPUTE_BIT : (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	auto command_size = type == MARS_COMMAND_SIGNATURE_DISPATCH ? sizeof(VkDispatchIndirectCommand) : sizeof(VkDrawIndexedIndirectCommand);
	auto token_type = type == MARS_COMMAND_SIGNATURE_DISPATCH ? VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT : VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT;
	return make_executor(dev, type, false, pipeline_type, VK_NULL_HANDLE, VK_NULL_HANDLE, shader_type, 0u, 0u, command_size, token_type);
}

indirect_executor vk_indirect_executor_impl::create_with_constant(const device& dev, const pipeline& pipe) {
	auto* pipe_data = pipe.data.expect<vk_pipeline_data>();
	mars::logger::assert_(pipe_data->has_push_constants, vk_log_channel(), "create_with_constant requires a graphics pipeline with push constants");
	mars::logger::assert_(pipe_data->push_constant_count >= 2u, vk_log_channel(), "create_with_constant requires at least 2 DWORDs of push constants");
	mars::logger::assert_(pipe_data->pipeline != VK_NULL_HANDLE, vk_log_channel(), "create_with_constant requires a valid graphics pipeline");
	return make_executor(dev, MARS_COMMAND_SIGNATURE_DRAW_INDEXED, true, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_data->pipeline, pipe_data->layout, VK_SHADER_STAGE_VERTEX_BIT, 2u * sizeof(uint32_t), 2u * sizeof(uint32_t), 7u * sizeof(uint32_t), VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_EXT);
}

indirect_executor vk_indirect_executor_impl::create_with_constant_compute(const device& dev, const compute_pipeline& pipe) {
	auto* pipe_data = pipe.data.expect<vk_compute_pipeline_data>();
	mars::logger::assert_(pipe_data->has_push_constants, vk_log_channel(), "create_with_constant_compute requires a compute pipeline with push constants");
	mars::logger::assert_(pipe_data->push_constant_count >= 1u, vk_log_channel(), "create_with_constant_compute requires at least 1 DWORD of push constants");
	mars::logger::assert_(pipe_data->pipeline != VK_NULL_HANDLE, vk_log_channel(), "create_with_constant_compute requires a valid compute pipeline");
	return make_executor(dev, MARS_COMMAND_SIGNATURE_DISPATCH, true, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_data->pipeline, pipe_data->layout, pipe_data->push_constant_stage_flags, 1u * sizeof(uint32_t), 1u * sizeof(uint32_t), 4u * sizeof(uint32_t), VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_EXT);
}

void vk_indirect_executor_impl::record_dispatch(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	record_impl(ex, cmd, arg_buf, max_count, count_buf);
}

void vk_indirect_executor_impl::record_draw_indexed(const indirect_executor& ex, const command_buffer& cmd, const buffer& arg_buf, uint32_t max_count, const buffer* count_buf) {
	record_impl(ex, cmd, arg_buf, max_count, count_buf);
}

void vk_indirect_executor_impl::destroy(indirect_executor& ex) {
	auto* data = ex.data.expect<vk_indirect_executor_data>();
	
	for (auto& preprocess_entry : data->preprocess_buffers)
		destroy_preprocess_buffer(data->device_data, preprocess_entry);

	data->preprocess_buffers.clear();

	if (data->indirect_layout != VK_NULL_HANDLE)
		data->device_data->destroy_indirect_commands_layout(data->device_data->device, data->indirect_layout, nullptr);

	delete data;
	ex = {};
}
} // namespace mars::graphics::vk
