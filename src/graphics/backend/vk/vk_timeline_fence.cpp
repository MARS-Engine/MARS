#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_timeline_fence.hpp>

namespace mars::graphics::vk {
timeline_fence vk_timeline_fence_impl::create(const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_timeline_fence_data();
	data->device = device_data->device;

	VkSemaphoreTypeCreateInfo type_info = {};
	type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	type_info.initialValue = 0u;

	VkSemaphoreCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	create_info.pNext = &type_info;
	vk_expect<vkCreateSemaphore>(device_data->device, &create_info, nullptr, &data->semaphore);

	timeline_fence result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

uint64_t vk_timeline_fence_impl::alloc_value(const timeline_fence& _fence) {
	auto* data = _fence.data.expect<vk_timeline_fence_data>();
	return data->next_value.fetch_add(1u, std::memory_order_relaxed);
}

void vk_timeline_fence_impl::signal_on_queue(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value) {
	auto* fence_data = _fence.data.expect<vk_timeline_fence_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* queue_data = vk_get_queue(device_data, queue);

	VkTimelineSemaphoreSubmitInfo timeline_info = {};
	timeline_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timeline_info.signalSemaphoreValueCount = 1u;
	timeline_info.pSignalSemaphoreValues = &value;

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = &timeline_info;
	submit_info.signalSemaphoreCount = 1u;
	submit_info.pSignalSemaphores = &fence_data->semaphore;

	vk_expect<vkQueueSubmit>(queue_data->queue, 1u, &submit_info, VK_NULL_HANDLE);
}

void vk_timeline_fence_impl::wait_on_queue(const timeline_fence& _fence, const device& _device, mars_command_queue_type queue, uint64_t value) {
	auto* fence_data = _fence.data.expect<vk_timeline_fence_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* queue_data = vk_get_queue(device_data, queue);

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkTimelineSemaphoreSubmitInfo timeline_info = {};
	timeline_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	timeline_info.waitSemaphoreValueCount = 1u;
	timeline_info.pWaitSemaphoreValues = &value;

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = &timeline_info;
	submit_info.waitSemaphoreCount = 1u;
	submit_info.pWaitSemaphores = &fence_data->semaphore;
	submit_info.pWaitDstStageMask = &wait_stage;

	vk_expect<vkQueueSubmit>(queue_data->queue, 1u, &submit_info, VK_NULL_HANDLE);
}

void vk_timeline_fence_impl::cpu_wait(const timeline_fence& _fence, uint64_t value) {
	auto* data = _fence.data.expect<vk_timeline_fence_data>();
	VkSemaphoreWaitInfo wait_info = {};
	wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	wait_info.semaphoreCount = 1u;
	wait_info.pSemaphores = &data->semaphore;
	wait_info.pValues = &value;
	vk_expect<vkWaitSemaphores>(data->device, &wait_info, UINT64_MAX);
}

uint64_t vk_timeline_fence_impl::completed_value(const timeline_fence& _fence) {
	auto* data = _fence.data.expect<vk_timeline_fence_data>();
	uint64_t value = 0u;
	vk_expect<vkGetSemaphoreCounterValue>(data->device, data->semaphore, &value);
	return value;
}

void vk_timeline_fence_impl::destroy(timeline_fence& _fence) {
	auto* data = _fence.data.expect<vk_timeline_fence_data>();
	if (data->semaphore != VK_NULL_HANDLE)
		vkDestroySemaphore(data->device, data->semaphore, nullptr);
	delete data;
	_fence = {};
}
} // namespace mars::graphics::vk
