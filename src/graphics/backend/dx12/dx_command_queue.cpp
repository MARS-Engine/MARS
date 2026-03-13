#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_command_queue.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel dx12_channel("dx12");

static void create_queue(dx_device_data* device_data, D3D12_COMMAND_LIST_TYPE d3d_type, meta::type_erased_ptr& out_slot, const char* label) {
	auto data = new dx_command_queue_data();

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = d3d_type;
	dx_expect<&ID3D12Device::CreateCommandQueue>(device_data->device.Get(), &queue_desc, IID_PPV_ARGS(&data->cmd_queue));

	dx_expect<&ID3D12Device::CreateFence>(device_data->device.Get(), 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&data->fence));

	data->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	logger::error_if(!data->fence_event, dx12_channel, "{} CreateEvent failed", label);

	logger::log(dx12_channel, "{} queue created", label);
	out_slot.store(data);
}

static void destroy_queue(meta::type_erased_ptr& slot, const char* label) {
	auto data = slot.expect<dx_command_queue_data>();
	if (data->fence_event)
		CloseHandle(data->fence_event);
	delete data;
	slot = static_cast<dx_command_queue_data*>(nullptr);
	logger::log(dx12_channel, "{} queue destroyed", label);
}

void dx_command_queue_impl::dx_command_queue_create(device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	create_queue(device_data, D3D12_COMMAND_LIST_TYPE_DIRECT, device_data->command_queue_data, "DIRECT");
}

void dx_command_queue_impl::dx_command_queue_destroy(device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	destroy_queue(device_data->command_queue_data, "DIRECT");
}

void dx_command_queue_impl::dx_compute_queue_create(device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	create_queue(device_data, D3D12_COMMAND_LIST_TYPE_COMPUTE, device_data->compute_queue_data, "COMPUTE");
}

void dx_command_queue_impl::dx_compute_queue_destroy(device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	destroy_queue(device_data->compute_queue_data, "COMPUTE");
}

void dx_command_queue_impl::dx_copy_queue_create(device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	create_queue(device_data, D3D12_COMMAND_LIST_TYPE_COPY, device_data->copy_queue_data, "COPY");
}

void dx_command_queue_impl::dx_copy_queue_destroy(device& _device) {
	auto device_data = _device.data.expect<dx_device_data>();
	destroy_queue(device_data->copy_queue_data, "COPY");
}
} // namespace mars::graphics::dx
