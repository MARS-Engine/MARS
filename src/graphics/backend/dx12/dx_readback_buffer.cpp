#include "dx_internal.hpp"
#include <mars/graphics/backend/dx12/dx_readback_buffer.hpp>

#include <mars/debug/logger.hpp>

namespace mars::graphics::dx {
static log_channel rb_channel("dx12");

readback_buffer dx_readback_buffer_impl::create(const device& dev, size_t slot_size, uint32_t num_slots) {
	auto dev_data = dev.data.expect<dx_device_data>();
	auto data = new dx_readback_buffer_data();

	data->slot_size = slot_size;
	data->num_slots = num_slots;

	const size_t total_size = slot_size * static_cast<size_t>(num_slots);

	D3D12_HEAP_PROPERTIES heap_props = {};
	heap_props.Type = D3D12_HEAP_TYPE_READBACK;

	D3D12_RESOURCE_DESC res_desc = {};
	res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	res_desc.Width = static_cast<UINT64>(total_size);
	res_desc.Height = 1;
	res_desc.DepthOrArraySize = 1;
	res_desc.MipLevels = 1;
	res_desc.Format = DXGI_FORMAT_UNKNOWN;
	res_desc.SampleDesc = {1, 0};
	res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = dx_expect<&ID3D12Device::CreateCommittedResource>(
		dev_data->device.Get(),
		&heap_props,
		D3D12_HEAP_FLAG_NONE,
		&res_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&data->readback_resource)
	);

	D3D12_RANGE read_range = {0, total_size};
	hr = dx_expect<&ID3D12Resource::Map>(data->readback_resource.Get(), 0, &read_range, reinterpret_cast<void**>(&data->mapped_ptr));

	hr = dx_expect<&ID3D12Device::CreateCommandAllocator>(dev_data->device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&data->allocator));

	hr = dx_expect<&ID3D12Device::CreateCommandList>(dev_data->device.Get(), 0, D3D12_COMMAND_LIST_TYPE_DIRECT, data->allocator.Get(), nullptr, IID_PPV_ARGS(&data->cmd_list));

	hr = dx_expect<&ID3D12Device::CreateFence>(dev_data->device.Get(), 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&data->fence));

	data->wait_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	logger::error_if(!data->wait_event, rb_channel, "readback CreateEvent failed");

	data->direct_queue = dx_get_queue(dev_data, MARS_COMMAND_QUEUE_DIRECT)->cmd_queue.Get();

	readback_buffer rb;
	rb.engine = dev.engine;
	rb.data.store(data);
	return rb;
}

readback_ticket dx_readback_buffer_impl::schedule(readback_buffer& rb, const buffer& src, size_t src_offset, size_t size) {
	auto data = rb.data.expect<dx_readback_buffer_data>();
	auto src_data = src.data.expect<dx_buffer_data>();

	if (data->in_flight.size() >= static_cast<size_t>(data->num_slots)) {
		auto& oldest = data->in_flight.front();
		if (data->fence->GetCompletedValue() < oldest.fence_value) {
			data->fence->SetEventOnCompletion(oldest.fence_value, data->wait_event);
			WaitForSingleObjectEx(data->wait_event, INFINITE, FALSE);
		}
		data->in_flight.pop_front();
	}

	const uint32_t slot = data->current_slot;
	data->current_slot = (data->current_slot + 1) % data->num_slots;

	const UINT64 dst_offset = static_cast<UINT64>(slot) * static_cast<UINT64>(data->slot_size);

	data->cmd_list->CopyBufferRegion(
		data->readback_resource.Get(),
		dst_offset,
		src_data->resource.Get(),
		static_cast<UINT64>(src_offset),
		static_cast<UINT64>(size)
	);

	data->pending_work = true;

	readback_ticket ticket;
	ticket.slot_index = slot;
	ticket.fence_value = 0;
	ticket.data_size = size;
	return ticket;
}

uint64_t dx_readback_buffer_impl::flush(readback_buffer& rb) {
	auto data = rb.data.expect<dx_readback_buffer_data>();
	if (!data->pending_work)
		return 0;

	data->cmd_list->Close();

	ID3D12CommandList* lists[] = {data->cmd_list.Get()};
	data->direct_queue->ExecuteCommandLists(1, lists);

	const UINT64 fv = data->next_fence_value++;
	data->direct_queue->Signal(data->fence.Get(), fv);

	dx_readback_buffer_data::PendingBatch batch;
	batch.slot_index = (data->current_slot == 0 ? data->num_slots : data->current_slot) - 1;
	batch.data_size = data->slot_size;
	batch.fence_value = fv;
	data->in_flight.push_back(batch);

	data->pending_work = false;

	data->fence->SetEventOnCompletion(fv, data->wait_event);
	WaitForSingleObjectEx(data->wait_event, INFINITE, FALSE);

	data->allocator->Reset();
	data->cmd_list->Reset(data->allocator.Get(), nullptr);

	return fv;
}

bool dx_readback_buffer_impl::try_read(const readback_buffer& rb, const readback_ticket& ticket, const void** out_ptr) {
	auto data = rb.data.expect<dx_readback_buffer_data>();
	if (data->fence->GetCompletedValue() < ticket.fence_value)
		return false;
	*out_ptr = data->mapped_ptr + static_cast<size_t>(ticket.slot_index) * data->slot_size;
	return true;
}

void dx_readback_buffer_impl::release_slot(readback_buffer& rb, const readback_ticket& ticket) {
	auto data = rb.data.expect<dx_readback_buffer_data>();
	while (!data->in_flight.empty())
		if (data->fence->GetCompletedValue() >= data->in_flight.front().fence_value)
			data->in_flight.pop_front();
		else
			break;
	(void)ticket;
}

void dx_readback_buffer_impl::destroy(readback_buffer& rb) {
	auto data = rb.data.expect<dx_readback_buffer_data>();

	if (data->pending_work)
		flush(rb);

	if (!data->in_flight.empty()) {
		const UINT64 last_fv = data->in_flight.back().fence_value;
		if (data->fence->GetCompletedValue() < last_fv) {
			data->fence->SetEventOnCompletion(last_fv, data->wait_event);
			WaitForSingleObjectEx(data->wait_event, INFINITE, FALSE);
		}
		data->in_flight.clear();
	}

	if (data->readback_resource && data->mapped_ptr) {
		D3D12_RANGE written = {0, 0};
		data->readback_resource->Unmap(0, &written);
		data->mapped_ptr = nullptr;
	}

	if (data->wait_event) {
		CloseHandle(data->wait_event);
		data->wait_event = nullptr;
	}

	delete data;
	rb = {};
}
} // namespace mars::graphics::dx
