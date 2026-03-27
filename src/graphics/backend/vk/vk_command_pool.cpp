#include <mars/graphics/backend/vk/vk_command_pool.hpp>

#include "vk_internal.hpp"

#include <algorithm>

namespace mars::graphics::vk {
command_pool vk_command_pool_impl::vk_compute_command_pool_create(const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_command_pool_data();

	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = device_data->compute_queue.family_index;
	vk_expect<vkCreateCommandPool>(device_data->device, &create_info, nullptr, &data->command_pool);

	command_pool result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

command_pool vk_command_pool_impl::vk_command_pool_create(const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_command_pool_data();

	VkCommandPoolCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = device_data->direct_queue.family_index;
	vk_expect<vkCreateCommandPool>(device_data->device, &create_info, nullptr, &data->command_pool);

	command_pool result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

std::vector<command_buffer> vk_command_pool_impl::vk_command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _n_command_buffers) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* pool_data = _command_pool.data.expect<vk_command_pool_data>();

	std::vector<VkCommandBuffer> vk_command_buffers(_n_command_buffers);
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = pool_data->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = static_cast<uint32_t>(_n_command_buffers);
	vk_expect<vkAllocateCommandBuffers>(device_data->device, &alloc_info, vk_command_buffers.data());

	std::vector<command_buffer> result;
	result.reserve(_n_command_buffers);
	for (size_t index = 0u; index < _n_command_buffers; ++index) {
		auto* command_buffer_data = new vk_command_buffer_data();
		command_buffer_data->command_buffer = vk_command_buffers[index];
		command_buffer_data->pool = pool_data;
		command_buffer_data->device_data = device_data;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vk_expect<vkCreateFence>(device_data->device, &fence_info, nullptr, &command_buffer_data->submit_fence);
		pool_data->command_buffers.push_back(command_buffer_data);

		command_buffer handle;
		handle.engine = _device.engine;
		handle.data.store(command_buffer_data);
		handle.buffer_index = index;
		result.push_back(handle);
	}
	return result;
}

void vk_command_pool_impl::vk_command_buffer_reset(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	if (command_buffer_data->submitted && command_buffer_data->submit_fence != VK_NULL_HANDLE) {
		vk_expect<vkWaitForFences>(command_buffer_data->device_data->device, 1u, &command_buffer_data->submit_fence, VK_TRUE, UINT64_MAX);
		vk_expect<vkResetFences>(command_buffer_data->device_data->device, 1u, &command_buffer_data->submit_fence);
		command_buffer_data->submitted = false;
	}
	vk_expect<vkResetCommandBuffer>(command_buffer_data->command_buffer, 0u);
	command_buffer_data->last_bound_framebuffer = nullptr;
	command_buffer_data->swapchain = nullptr;
	command_buffer_data->has_global_memory_barrier = false;
}

void vk_command_pool_impl::vk_command_buffer_record(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	command_buffer_data->has_global_memory_barrier = false;
	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vk_expect<vkBeginCommandBuffer>(command_buffer_data->command_buffer, &begin_info);
}

void vk_command_pool_impl::vk_command_buffer_record_end(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	vk_expect<vkEndCommandBuffer>(command_buffer_data->command_buffer);
}

void vk_command_pool_impl::vk_command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	vkCmdDraw(
		command_buffer_data->command_buffer,
		static_cast<uint32_t>(_params.vertex_count),
		static_cast<uint32_t>(_params.instance_count),
		static_cast<uint32_t>(_params.first_vertex),
		static_cast<uint32_t>(_params.first_instance)
	);
}

void vk_command_pool_impl::vk_command_buffer_draw_indexed(const command_buffer& _command_buffer, const command_buffer_draw_indexed_params& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	vkCmdDrawIndexed(
		command_buffer_data->command_buffer,
		static_cast<uint32_t>(_params.index_count),
		static_cast<uint32_t>(_params.instance_count),
		static_cast<uint32_t>(_params.first_index),
		static_cast<int32_t>(_params.vertex_offset),
		static_cast<uint32_t>(_params.first_instance)
	);
}

void vk_command_pool_impl::vk_command_buffer_dispatch(const command_buffer& _command_buffer, const command_buffer_dispatch_params& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	vkCmdDispatch(
		command_buffer_data->command_buffer,
		static_cast<uint32_t>(_params.group_count_x),
		static_cast<uint32_t>(_params.group_count_y),
		static_cast<uint32_t>(_params.group_count_z)
	);
}

void vk_command_pool_impl::vk_command_buffer_set_push_constants(const command_buffer& _command_buffer, const pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	auto* pipeline_data = _pipeline.data.expect<vk_pipeline_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	if (!pipeline_data->has_push_constants || _values == nullptr)
		return;

	const size_t clamped_count = std::min(_count, pipeline_data->push_constant_count);
	const uint32_t value_bytes = static_cast<uint32_t>(clamped_count * sizeof(uint32_t));
	if (value_bytes == 0u)
		return;

	vkCmdPushConstants(command_buffer_data->command_buffer, pipeline_data->layout, pipeline_data->push_constant_stage_flags, 0u, value_bytes, _values);
}

void vk_command_pool_impl::vk_command_buffer_set_compute_push_constants(const command_buffer& _command_buffer, const compute_pipeline& _pipeline, const uint32_t* _values, size_t _count) {
	auto* pipeline_data = _pipeline.data.expect<vk_compute_pipeline_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	if (!pipeline_data->has_push_constants || _values == nullptr)
		return;

	const size_t clamped_count = std::min(_count, pipeline_data->push_constant_count);
	const uint32_t value_bytes = static_cast<uint32_t>(clamped_count * sizeof(uint32_t));
	if (value_bytes == 0u)
		return;

	vkCmdPushConstants(command_buffer_data->command_buffer, pipeline_data->layout, pipeline_data->push_constant_stage_flags, 0u, value_bytes, _values);
}

void vk_command_pool_impl::vk_command_buffer_memory_barrier(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	vk_cmd_pipeline_memory_barrier2(
		command_buffer_data->command_buffer,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT
	);
	command_buffer_data->has_global_memory_barrier = true;
}

void vk_command_pool_impl::vk_command_pool_destroy(command_pool& _command_pool, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* pool_data = _command_pool.data.expect<vk_command_pool_data>();

	for (auto* command_buffer_data : pool_data->command_buffers) {
		if (!command_buffer_data)
			continue;
		if (command_buffer_data->submitted && command_buffer_data->submit_fence != VK_NULL_HANDLE)
			vk_expect<vkWaitForFences>(device_data->device, 1u, &command_buffer_data->submit_fence, VK_TRUE, UINT64_MAX);
		if (command_buffer_data->submit_fence != VK_NULL_HANDLE)
			vkDestroyFence(device_data->device, command_buffer_data->submit_fence, nullptr);
		delete command_buffer_data;
	}
	pool_data->command_buffers.clear();

	if (pool_data->command_pool != VK_NULL_HANDLE)
		vkDestroyCommandPool(device_data->device, pool_data->command_pool, nullptr);
	delete pool_data;
	_command_pool = {};
}

void vk_command_pool_impl::vk_command_buffer_begin_event(const command_buffer& _command_buffer, std::string_view _name) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	if (!command_buffer_data->device_data->cmd_begin_debug_label || _name.empty())
		return;

	VkDebugUtilsLabelEXT label = {};
	label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label.pLabelName = _name.data();
	command_buffer_data->device_data->cmd_begin_debug_label(command_buffer_data->command_buffer, &label);
}

void vk_command_pool_impl::vk_command_buffer_end_event(const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	if (command_buffer_data->device_data->cmd_end_debug_label)
		command_buffer_data->device_data->cmd_end_debug_label(command_buffer_data->command_buffer);
}

void vk_command_pool_impl::vk_command_buffer_trace_rays(const command_buffer& _command_buffer, const mars::ray_tracing_pipeline& _pipeline, const mars::rt_dispatch_regions& _regions, uint32_t _width, uint32_t _height, uint32_t _depth) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();

	auto to_vk = [](const mars::rt_dispatch_region& _region) -> VkStridedDeviceAddressRegionKHR {
		return { _region.address, _region.stride, _region.size };
	};
	const VkStridedDeviceAddressRegionKHR raygen = to_vk(_regions.raygen);
	const VkStridedDeviceAddressRegionKHR miss = to_vk(_regions.miss);
	const VkStridedDeviceAddressRegionKHR hit = to_vk(_regions.hit);
	const VkStridedDeviceAddressRegionKHR callable = to_vk(_regions.callable);

	command_buffer_data->device_data->cmd_trace_rays(command_buffer_data->command_buffer, &raygen, &miss, &hit, &callable, _width, _height, _depth);
}
} // namespace mars::graphics::vk
