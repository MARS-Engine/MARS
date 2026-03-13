#include "dx_internal.hpp"
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/dx12/dx_timeline_fence.hpp>
#include <mars/graphics/functional/graphics_engine.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

timeline_fence dx_timeline_fence_impl::create(const device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	auto data = new dx_timeline_fence_data();

	dx_expect<&ID3D12Device::CreateFence>(device_data->device.Get(), 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&data->fence));

	data->event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	logger::error_if(!data->event, dx12_channel, "timeline_fence CreateEvent failed");

	logger::log(dx12_channel, "timeline_fence created");

	timeline_fence result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

uint64_t dx_timeline_fence_impl::alloc_value(const timeline_fence& _fence) {
	auto data = _fence.data.expect<dx_timeline_fence_data>();
	return data->next_value.fetch_add(1, std::memory_order_relaxed);
}

void dx_timeline_fence_impl::signal_on_queue(
	const timeline_fence& _fence,
	const device& _device,
	mars_command_queue_type queue,
	uint64_t value
) {
	auto data = _fence.data.expect<dx_timeline_fence_data>();
	auto device_data = _device.data.expect<dx_device_data>();
	auto queue_data = dx_get_queue(device_data, queue);

	dx_expect<&ID3D12CommandQueue::Signal>(queue_data->cmd_queue.Get(), data->fence.Get(), value);
}

void dx_timeline_fence_impl::wait_on_queue(
	const timeline_fence& _fence,
	const device& _device,
	mars_command_queue_type queue,
	uint64_t value
) {
	auto data = _fence.data.expect<dx_timeline_fence_data>();
	auto device_data = _device.data.expect<dx_device_data>();
	auto queue_data = dx_get_queue(device_data, queue);

	dx_expect<&ID3D12CommandQueue::Wait>(queue_data->cmd_queue.Get(), data->fence.Get(), value);
}

void dx_timeline_fence_impl::cpu_wait(const timeline_fence& _fence, uint64_t value) {
	auto data = _fence.data.expect<dx_timeline_fence_data>();

	if (data->fence->GetCompletedValue() < value) {
		dx_expect<&ID3D12Fence::SetEventOnCompletion>(data->fence.Get(), value, data->event);

		WaitForSingleObjectEx(data->event, INFINITE, FALSE);
	}
}

uint64_t dx_timeline_fence_impl::completed_value(const timeline_fence& _fence) {
	auto data = _fence.data.expect<dx_timeline_fence_data>();
	return data->fence->GetCompletedValue();
}

void dx_timeline_fence_impl::destroy(timeline_fence& _fence) {
	auto data = _fence.data.expect<dx_timeline_fence_data>();

	if (data->event) {
		CloseHandle(data->event);
		data->event = nullptr;
	}
	delete data;
	_fence = {};
	logger::log(dx12_channel, "timeline_fence destroyed");
}
} // namespace mars::graphics::dx
