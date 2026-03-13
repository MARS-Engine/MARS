#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_upload_ring.hpp>

#include <cstring>

namespace mars::graphics::vk {
namespace {
void reset_upload_ring(vk_upload_ring_data* data) {
	vk_expect<vkResetFences>(data->device_data->device, 1u, &data->fence);
	vk_expect<vkResetCommandPool>(data->device_data->device, data->command_pool, 0u);
	data->write_head = 0u;
	data->recording = false;
}
} // namespace

upload_ring vk_upload_ring_impl::create(const device& _device, size_t capacity) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_upload_ring_data();
	data->device_data = device_data;
	data->queue = &device_data->direct_queue;
	data->capacity = capacity;

	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = capacity;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_expect<vkCreateBuffer>(device_data->device, &buffer_info, nullptr, &data->upload_buffer);

	VkMemoryRequirements requirements = {};
	vkGetBufferMemoryRequirements(device_data->device, data->upload_buffer, &requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = vk_find_memory_type(
		device_data,
		requirements.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	vk_expect<vkAllocateMemory>(device_data->device, &alloc_info, nullptr, &data->upload_memory);
	vk_expect<vkBindBufferMemory>(device_data->device, data->upload_buffer, data->upload_memory, 0u);
	vk_expect<vkMapMemory>(device_data->device, data->upload_memory, 0u, capacity, 0u, reinterpret_cast<void**>(&data->mapped_ptr));

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

	upload_ring result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void vk_upload_ring_impl::schedule(upload_ring& _ring, const buffer& dst, size_t dst_offset, const void* src, size_t size) {
	auto* data = _ring.data.expect<vk_upload_ring_data>();
	auto* dst_data = dst.data.expect<vk_buffer_data>();
	if (!src || size == 0u)
		return;

	const size_t aligned_size = vk_align_up(size, size_t{256});
	if (aligned_size > data->capacity) {
		mars::logger::error(vk_log_channel(), "upload_ring request exceeds capacity");
		return;
	}
	if (data->write_head + aligned_size > data->capacity)
		flush(_ring);

	if (!data->recording) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vk_expect<vkBeginCommandBuffer>(data->command_buffer, &begin_info);
		data->recording = true;
	}

	std::memcpy(data->mapped_ptr + data->write_head, src, size);

	VkBufferCopy copy = {};
	copy.srcOffset = data->write_head;
	copy.dstOffset = dst_offset;
	copy.size = size;
	vkCmdCopyBuffer(data->command_buffer, data->upload_buffer, dst_data->buffer, 1u, &copy);
	data->write_head += aligned_size;
}

uint64_t vk_upload_ring_impl::flush(upload_ring& _ring) {
	auto* data = _ring.data.expect<vk_upload_ring_data>();
	if (!data->recording)
		return 0u;

	const uint64_t fence_value = data->next_fence_value++;
	vk_expect<vkEndCommandBuffer>(data->command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1u;
	submit_info.pCommandBuffers = &data->command_buffer;
	vk_expect<vkQueueSubmit>(data->queue->queue, 1u, &submit_info, data->fence);
	vk_expect<vkWaitForFences>(data->device_data->device, 1u, &data->fence, VK_TRUE, UINT64_MAX);
	reset_upload_ring(data);
	return fence_value;
}

void vk_upload_ring_impl::cpu_wait(const upload_ring&, uint64_t) {
}

void vk_upload_ring_impl::destroy(upload_ring& _ring) {
	auto* data = _ring.data.expect<vk_upload_ring_data>();
	if (data->recording)
		flush(_ring);
	if (data->mapped_ptr)
		vkUnmapMemory(data->device_data->device, data->upload_memory);
	if (data->fence != VK_NULL_HANDLE)
		vkDestroyFence(data->device_data->device, data->fence, nullptr);
	if (data->command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(data->device_data->device, data->command_pool, nullptr);
	vk_destroy_buffer(data->device_data, data->upload_buffer, data->upload_memory);
	delete data;
	_ring = {};
}
} // namespace mars::graphics::vk
