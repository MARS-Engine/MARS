#include <mars/graphics/backend/vk/vk_buffer.hpp>

#include "vk_internal.hpp"

namespace mars::graphics::vk {
namespace {
void transition_for_input_binding(vk_command_buffer_data* _command_buffer_data, vk_buffer_data* _buffer_data, VkPipelineStageFlags _target_stage, VkAccessFlags _target_access) {
	if (_buffer_data->current_stage == _target_stage && _buffer_data->current_access == _target_access)
		return;

	vk_cmd_pipeline_buffer_barrier2(_command_buffer_data->command_buffer, _buffer_data->current_stage, _buffer_data->current_access, _target_stage, _target_access, _buffer_data->buffer, 0u, _buffer_data->size);
	_buffer_data->current_stage = _target_stage;
	_buffer_data->current_access = _target_access;
}

VkBufferUsageFlags buffer_usage_flags_from_params(const buffer_create_params& _params) {
	VkBufferUsageFlags usage =
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_VERTEX) != 0u)
		usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_INDEX) != 0u)
		usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if ((_params.buffer_type & (MARS_BUFFER_TYPE_VERTEX | MARS_BUFFER_TYPE_INDEX)) != 0u)
		usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_ACCELERATION_STRUCTURE_STORAGE) != 0u)
		usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_ACCELERATION_STRUCTURE_BUILD_INPUT) != 0u)
		usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_UNIFORM) != 0u)
		usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_UNIFORM_TEXEL_BUFFER) != 0u)
		usage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_UNORDERED_ACCESS) != 0u)
		usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_SHADER_RESOURCE) != 0u)
		usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_SHADER_BINDING_TABLE) != 0u)
		usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
	if ((_params.buffer_type & MARS_BUFFER_TYPE_STRUCTURED) != 0u)
		usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (_params.stride != 0u)
		usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	return usage;
}

void create_buffer_resource(vk_device_data* _device_data, vk_buffer_data* _data, const buffer_create_params& _params) {
	VkBufferCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = _params.allocated_size;
	create_info.usage = _data->usage;
	create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_expect<vkCreateBuffer>(_device_data->device, &create_info, nullptr, &_data->buffer);

	VkMemoryRequirements memory_requirements = {};
	vkGetBufferMemoryRequirements(_device_data->device, _data->buffer, &memory_requirements);

	const VkMemoryPropertyFlags properties = _data->host_visible
												 ? (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
												 : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkMemoryAllocateFlagsInfo allocate_flags = {};
	allocate_flags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocate_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocate_info = {};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.pNext = &allocate_flags;
	allocate_info.allocationSize = memory_requirements.size;
	allocate_info.memoryTypeIndex = vk_find_memory_type(_device_data, memory_requirements.memoryTypeBits, properties);
	vk_expect<vkAllocateMemory>(_device_data->device, &allocate_info, nullptr, &_data->memory);
	vk_expect<vkBindBufferMemory>(_device_data->device, _data->buffer, _data->memory, 0u);

	VkBufferDeviceAddressInfo address_info = {};
	address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	address_info.buffer = _data->buffer;
	_data->device_address = vkGetBufferDeviceAddress(_device_data->device, &address_info);
}

void update_bindless_buffer_descriptor(vk_device_data* _device_data, uint32_t _binding_index, VkDescriptorType _descriptor_type, VkBuffer _buffer_handle, VkDeviceSize _size) {
	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = _buffer_handle;
	buffer_info.offset = 0u;
	buffer_info.range = _size;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = _device_data->bindless_set;
	write.dstBinding = 0u;
	write.dstArrayElement = _binding_index;
	write.descriptorCount = 1u;
	write.descriptorType = _descriptor_type;
	write.pBufferInfo = &buffer_info;
	vkUpdateDescriptorSets(_device_data->device, 1u, &write, 0u, nullptr);
}

} // namespace

buffer vk_buffer_impl::vk_buffer_create(const device& _device, const buffer_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_buffer_data();
	data->size = _params.allocated_size;
	data->host_visible = (_params.buffer_property & MARS_BUFFER_PROPERTY_HOST_VISIBLE) != 0u;
	data->usage = buffer_usage_flags_from_params(_params);
	create_buffer_resource(device_data, data, _params);

	const bool has_srv = (_params.buffer_type & MARS_BUFFER_TYPE_SHADER_RESOURCE) != 0u || _params.stride != 0u;
	const bool has_uav = (_params.buffer_type & MARS_BUFFER_TYPE_UNORDERED_ACCESS) != 0u;
	if (has_srv) {
		data->srv_bindless_idx = vk_allocate_bindless_srv_slot(device_data);
		update_bindless_buffer_descriptor(device_data, data->srv_bindless_idx, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data->buffer, data->size);
	}
	if (has_uav) {
		data->uav_bindless_idx = vk_allocate_bindless_uav_range(device_data, 1u);
		update_bindless_buffer_descriptor(device_data, data->uav_bindless_idx, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data->buffer, data->size);
	}

	buffer result;
	result.engine = _device.engine;
	result.data.store(data);
	result.allocated_size = _params.allocated_size;
	return result;
}

void vk_buffer_impl::vk_buffer_bind(buffer& _buffer, const command_buffer& _command_buffer) {
	auto* buffer_data = _buffer.data.expect<vk_buffer_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	transition_for_input_binding(
		command_buffer_data,
		buffer_data,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
	);
	const VkDeviceSize offset = 0u;
	vkCmdBindVertexBuffers(command_buffer_data->command_buffer, 0u, 1u, &buffer_data->buffer, &offset);
}

void vk_buffer_impl::vk_buffer_bind_index(buffer& _buffer, const command_buffer& _command_buffer) {
	auto* buffer_data = _buffer.data.expect<vk_buffer_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	transition_for_input_binding(
		command_buffer_data,
		buffer_data,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		VK_ACCESS_INDEX_READ_BIT
	);
	vkCmdBindIndexBuffer(command_buffer_data->command_buffer, buffer_data->buffer, 0u, VK_INDEX_TYPE_UINT32);
}

void vk_buffer_impl::vk_buffer_copy(buffer& _buffer, buffer& _src_buffer, const command_buffer& _command_buffer, size_t _offset) {
	auto* dst = _buffer.data.expect<vk_buffer_data>();
	auto* src = _src_buffer.data.expect<vk_buffer_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	VkBufferCopy copy = {};
	copy.srcOffset = 0u;
	copy.dstOffset = _offset;
	copy.size = src->size;
	vkCmdCopyBuffer(command_buffer_data->command_buffer, src->buffer, dst->buffer, 1u, &copy);
}

void vk_buffer_impl::vk_buffer_transition(const command_buffer& _command_buffer, const buffer& _buffer, mars_buffer_state _state) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* data = _buffer.data.expect<vk_buffer_data>();
	const auto target = vk_buffer_barrier_state_from_mars(_state);
	if (data->current_access == target.access && data->current_stage == target.stage)
		return;

	if (_state == MARS_BUFFER_STATE_INDIRECT_ARGUMENT && command_buffer_data->last_bound_framebuffer != nullptr) {
		if (data->host_visible || command_buffer_data->has_global_memory_barrier) {
			data->current_access = target.access;
			data->current_stage = target.stage;
			return;
		}
	}

	vk_cmd_pipeline_buffer_barrier2(
		command_buffer_data->command_buffer,
		data->current_stage,
		data->current_access,
		target.stage,
		target.access,
		data->buffer,
		0u,
		data->size
	);
	data->current_access = target.access;
	data->current_stage = target.stage;
}

void* vk_buffer_impl::vk_buffer_map(buffer& _buffer, const device& _device, size_t, size_t _offset) {
	auto* data = _buffer.data.expect<vk_buffer_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	if (!data->host_visible)
		return nullptr;
	if (!data->mapped_ptr)
		vk_expect<vkMapMemory>(device_data->device, data->memory, 0u, data->size, 0u, &data->mapped_ptr);
	return static_cast<uint8_t*>(data->mapped_ptr) + _offset;
}

void vk_buffer_impl::vk_buffer_unmap(buffer& _buffer, const device& _device) {
	auto* data = _buffer.data.expect<vk_buffer_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	if (data->mapped_ptr) {
		vkUnmapMemory(device_data->device, data->memory);
		data->mapped_ptr = nullptr;
	}
	if (data->host_visible) {
		data->current_access = VK_ACCESS_HOST_WRITE_BIT;
		data->current_stage = VK_PIPELINE_STAGE_HOST_BIT;
	}
}

void vk_buffer_impl::vk_buffer_destroy(buffer& _buffer, const device& _device) {
	auto* data = _buffer.data.expect<vk_buffer_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	vk_release_bindless_srv_slot(device_data, data->srv_bindless_idx);
	vk_release_bindless_uav_range(device_data, data->uav_bindless_idx, data->uav_bindless_idx == std::numeric_limits<uint32_t>::max() ? 0u : 1u);
	if (data->mapped_ptr)
		vkUnmapMemory(device_data->device, data->memory);
	vk_destroy_buffer(device_data, data->buffer, data->memory);
	delete data;
	_buffer = {};
}

uint32_t vk_buffer_impl::vk_buffer_get_uav_index(const buffer& _buffer) {
	auto* data = _buffer.data.expect<vk_buffer_data>();
	return data->uav_bindless_idx;
}

uint32_t vk_buffer_impl::vk_buffer_get_srv_index(const buffer& _buffer) {
	auto* data = _buffer.data.expect<vk_buffer_data>();
	return data->srv_bindless_idx;
}

uint64_t vk_buffer_impl::vk_buffer_get_device_address(const buffer& _buffer) {
	return _buffer.data.expect<vk_buffer_data>()->device_address;
}
} // namespace mars::graphics::vk
