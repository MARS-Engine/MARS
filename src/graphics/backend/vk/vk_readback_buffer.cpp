#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_readback_buffer.hpp>

#include <algorithm>

namespace mars::graphics::vk {
namespace {
void begin_if_needed(vk_readback_buffer_data* data) {
	if (data->pending)
		return;

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vk_expect<vkBeginCommandBuffer>(data->command_buffer, &begin_info);
	data->pending = true;
}

void reset_readback(vk_readback_buffer_data* data) {
	vk_expect<vkResetFences>(data->device_data->device, 1u, &data->fence);
	vk_expect<vkResetCommandPool>(data->device_data->device, data->command_pool, 0u);
	data->pending = false;
}
} // namespace

readback_buffer vk_readback_buffer_impl::create(const device& dev, size_t slot_size, uint32_t num_slots) {
	auto* device_data = dev.data.expect<vk_device_data>();
	auto* data = new vk_readback_buffer_data();
	data->device_data = device_data;
	data->queue = &device_data->direct_queue;
	data->slot_size = slot_size;
	data->num_slots = num_slots;

	const VkDeviceSize total_size = slot_size * static_cast<VkDeviceSize>(num_slots);

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = total_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_expect<vkCreateBuffer>(device_data->device, &buffer_info, nullptr, &data->readback_buffer);

	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements(device_data->device, data->readback_buffer, &requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = vk_find_memory_type(
		device_data,
		requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	vk_expect<vkAllocateMemory>(device_data->device, &alloc_info, nullptr, &data->readback_memory);
	vk_expect<vkBindBufferMemory>(device_data->device, data->readback_buffer, data->readback_memory, 0u);
	vk_expect<vkMapMemory>(device_data->device, data->readback_memory, 0u, total_size, 0u, reinterpret_cast<void**>(&data->mapped_ptr));

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = data->queue->family_index;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vk_expect<vkCreateCommandPool>(device_data->device, &pool_info, nullptr, &data->command_pool);

	VkCommandBufferAllocateInfo cmd_alloc = {};
	cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_alloc.commandPool = data->command_pool;
	cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_alloc.commandBufferCount = 1u;
	vk_expect<vkAllocateCommandBuffers>(device_data->device, &cmd_alloc, &data->command_buffer);

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vk_expect<vkCreateFence>(device_data->device, &fence_info, nullptr, &data->fence);

	readback_buffer result;
	result.engine = dev.engine;
	result.data.store(data);
	return result;
}

readback_ticket vk_readback_buffer_impl::schedule(readback_buffer& rb, const buffer& src, size_t src_offset, size_t size) {
	auto* data = rb.data.expect<vk_readback_buffer_data>();
	auto* src_data = src.data.expect<vk_buffer_data>();

	begin_if_needed(data);

	const uint32_t slot = data->current_slot;
	data->current_slot = (data->current_slot + 1u) % (std::max)(data->num_slots, 1u);

	VkBufferCopy copy = {};
	copy.srcOffset = src_offset;
	copy.dstOffset = static_cast<VkDeviceSize>(slot) * data->slot_size;
	copy.size = size;
	vkCmdCopyBuffer(data->command_buffer, src_data->buffer, data->readback_buffer, 1u, &copy);

	readback_ticket ticket;
	ticket.slot_index = slot;
	ticket.fence_value = data->next_fence_value;
	ticket.data_size = size;
	data->in_flight.push_back({slot, size, ticket.fence_value});
	return ticket;
}

uint64_t vk_readback_buffer_impl::flush(readback_buffer& rb) {
	auto* data = rb.data.expect<vk_readback_buffer_data>();
	if (!data->pending)
		return 0u;

	const uint64_t fence_value = data->next_fence_value++;
	vk_expect<vkEndCommandBuffer>(data->command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1u;
	submit_info.pCommandBuffers = &data->command_buffer;
	vk_expect<vkQueueSubmit>(data->queue->queue, 1u, &submit_info, data->fence);
	vk_expect<vkWaitForFences>(data->device_data->device, 1u, &data->fence, VK_TRUE, UINT64_MAX);
	reset_readback(data);
	return fence_value;
}

bool vk_readback_buffer_impl::try_read(const readback_buffer& rb, const readback_ticket& ticket, const void** out_ptr) {
	auto* data = rb.data.expect<vk_readback_buffer_data>();
	*out_ptr = data->mapped_ptr + static_cast<size_t>(ticket.slot_index) * data->slot_size;
	return true;
}

void vk_readback_buffer_impl::release_slot(readback_buffer& rb, const readback_ticket& ticket) {
	auto* data = rb.data.expect<vk_readback_buffer_data>();
	while (!data->in_flight.empty() && data->in_flight.front().fence_value <= ticket.fence_value)
		data->in_flight.pop_front();
}

void vk_readback_buffer_impl::destroy(readback_buffer& rb) {
	auto* data = rb.data.expect<vk_readback_buffer_data>();
	if (data->pending)
		flush(rb);
	if (data->mapped_ptr)
		vkUnmapMemory(data->device_data->device, data->readback_memory);
	if (data->fence != VK_NULL_HANDLE)
		vkDestroyFence(data->device_data->device, data->fence, nullptr);
	if (data->command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(data->device_data->device, data->command_pool, nullptr);
	vk_destroy_buffer(data->device_data, data->readback_buffer, data->readback_memory);
	delete data;
	rb = {};
}
} // namespace mars::graphics::vk
