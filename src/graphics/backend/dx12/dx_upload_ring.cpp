#include "dx_internal.hpp"
#include <cstring>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/dx12/dx_upload_ring.hpp>
#include <mars/graphics/backend/upload_ring.hpp>

namespace mars::graphics::dx {
static log_channel ring_channel("upload_ring");

static void wait_for_upload_fence(dx_upload_ring_data* data, UINT64 fence_value) {
	if (fence_value == 0 || data->fence->GetCompletedValue() >= fence_value)
		return;
	data->fence->SetEventOnCompletion(fence_value, data->wait_event);
	WaitForSingleObjectEx(data->wait_event, INFINITE, FALSE);
}

static void retire_completed_upload_batches(dx_upload_ring_data* data) {
	const UINT64 completed = data->fence->GetCompletedValue();
	while (!data->in_flight.empty() && data->in_flight.front().fence_value <= completed)
		data->in_flight.pop_front();
}

static bool upload_region_overlaps(const dx_upload_ring_data* data, size_t start, size_t end) {
	for (const auto& batch : data->in_flight) {
		if (end <= batch.region_start || start >= batch.region_end)
			continue;
		return true;
	}
	return false;
}

mars::upload_ring dx_upload_ring_impl::create(const mars::device& _device, size_t capacity) {
	auto* device_data = dx_expect_backend_data(_device.data.get<dx_device_data>(), __func__, "device.data");
	auto* copy_qdata = dx_expect_backend_data(device_data->copy_queue_data.get<dx_command_queue_data>(), __func__, "device.copy_queue_data");

	auto* d = new dx_upload_ring_data();
	d->capacity = capacity;
	d->copy_queue = copy_qdata->cmd_queue.Get();

	D3D12_HEAP_PROPERTIES heap_props = {};
	heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = capacity;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = device_data->device->CreateCommittedResource(
	    &heap_props, D3D12_HEAP_FLAG_NONE, &desc,
	    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
	    IID_PPV_ARGS(&d->upload_buffer));
	logger::error_if(FAILED(hr), ring_channel, "upload ring: CreateCommittedResource failed (hr={:#x})", (unsigned long)hr);

	D3D12_RANGE read_range = {0, 0};
	d->upload_buffer->Map(0, &read_range, reinterpret_cast<void**>(&d->mapped_ptr));

	hr = device_data->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d->fence));
	logger::error_if(FAILED(hr), ring_channel, "upload ring: CreateFence failed (hr={:#x})", (unsigned long)hr);

	d->wait_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	logger::error_if(!d->wait_event, ring_channel, "upload ring: CreateEvent failed");

	hr = device_data->device->CreateCommandAllocator(
	    D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&d->allocator));
	logger::error_if(FAILED(hr), ring_channel, "upload ring: CreateCommandAllocator failed (hr={:#x})", (unsigned long)hr);

	hr = device_data->device->CreateCommandList(
	    0, D3D12_COMMAND_LIST_TYPE_COPY,
	    d->allocator.Get(), nullptr, IID_PPV_ARGS(&d->cmd_list));
	logger::error_if(FAILED(hr), ring_channel, "upload ring: CreateCommandList failed (hr={:#x})", (unsigned long)hr);

	logger::log(ring_channel, "upload ring created (capacity={}MB)", capacity / (1024 * 1024));

	mars::upload_ring result;
	result.engine = _device.engine;
	result.data.store(d);
	return result;
}

void dx_upload_ring_impl::schedule(mars::upload_ring& _ring, const mars::buffer& dst, size_t dst_offset, const void* src, size_t size) {
	auto* d = dx_expect_backend_data(_ring.data.get<dx_upload_ring_data>(), __func__, "upload_ring.data");
	auto* buf_data = dx_expect_backend_data(dst.data.get<dx_buffer_data>(), __func__, "buffer.data");

	if (!src || size == 0)
		return;

	constexpr size_t ALIGN = 256;
	const size_t aligned_size = (size + ALIGN - 1) & ~(ALIGN - 1);

	logger::error_if(aligned_size > d->capacity, ring_channel,
			 "upload ring schedule exceeds ring capacity (size={}, aligned={}, capacity={})",
			 size, aligned_size, d->capacity);
	assert(aligned_size <= d->capacity && "upload ring request exceeds capacity");

	logger::error_if(dst_offset + size > buf_data->size, ring_channel,
			 "upload ring schedule exceeds destination buffer (dst_offset={}, size={}, buffer_size={})",
			 dst_offset, size, buf_data->size);
	assert(dst_offset + size <= buf_data->size && "upload ring destination range out of bounds");

	logger::error_if(d->batch_start > d->write_head, ring_channel,
			 "upload ring batch state corrupted (batch_start={}, write_head={})",
			 d->batch_start, d->write_head);
	assert(d->batch_start <= d->write_head && "upload ring batch state corrupted");

	retire_completed_upload_batches(d);

	if (d->write_head + aligned_size > d->capacity) {
		if (d->state == dx_upload_ring_state::recording)
			flush(_ring);

		retire_completed_upload_batches(d);

		while (upload_region_overlaps(d, 0, aligned_size)) {
			wait_for_upload_fence(d, d->in_flight.front().fence_value);
			retire_completed_upload_batches(d);
		}
		d->write_head = 0;
		d->batch_start = 0;
	}

	const size_t offset = d->write_head;
	logger::error_if(upload_region_overlaps(d, offset, offset + aligned_size), ring_channel,
			 "upload ring scheduled overlapping in-flight region (offset={}, aligned={})",
			 offset, aligned_size);
	assert(!upload_region_overlaps(d, offset, offset + aligned_size) && "upload ring overlap with in-flight batch");

	std::memcpy(d->mapped_ptr + offset, src, size);

	d->cmd_list->CopyBufferRegion(
	    buf_data->resource.Get(), dst_offset,
	    d->upload_buffer.Get(), offset, size);

	d->write_head += aligned_size;
	d->state = dx_upload_ring_state::recording;
}

uint64_t dx_upload_ring_impl::flush(mars::upload_ring& _ring) {
	auto* d = dx_expect_backend_data(_ring.data.get<dx_upload_ring_data>(), __func__, "upload_ring.data");
	if (d->state != dx_upload_ring_state::recording)
		return 0;

	logger::error_if(d->batch_start > d->write_head, ring_channel,
			 "upload ring flush observed invalid batch range (batch_start={}, write_head={})",
			 d->batch_start, d->write_head);
	assert(d->batch_start <= d->write_head && "upload ring flush observed invalid batch range");

	d->cmd_list->Close();
	ID3D12CommandList* lists[] = {d->cmd_list.Get()};
	d->copy_queue->ExecuteCommandLists(1, lists);

	const UINT64 fv = d->next_fence_value++;
	d->copy_queue->Signal(d->fence.Get(), fv);

	d->in_flight.push_back({d->batch_start, d->write_head, fv});
	d->batch_start = d->write_head;
	d->state = dx_upload_ring_state::idle;

	wait_for_upload_fence(d, fv);
	retire_completed_upload_batches(d);

	d->allocator->Reset();
	d->cmd_list->Reset(d->allocator.Get(), nullptr);

	return fv;
}

void dx_upload_ring_impl::cpu_wait(const mars::upload_ring& _ring, uint64_t fence_value) {
	auto* d = dx_expect_backend_data(_ring.data.get<dx_upload_ring_data>(), __func__, "upload_ring.data");
	if (fence_value == 0)
		return;
	wait_for_upload_fence(d, fence_value);
	retire_completed_upload_batches(d);
}

void dx_upload_ring_impl::destroy(mars::upload_ring& _ring) {
	auto* d = dx_expect_backend_data(_ring.data.get<dx_upload_ring_data>(), __func__, "upload_ring.data");

	if (d->state == dx_upload_ring_state::recording)
		flush(_ring);

	if (!d->in_flight.empty())
		cpu_wait(_ring, d->in_flight.back().fence_value);

	if (d->upload_buffer)
		d->upload_buffer->Unmap(0, nullptr);

	if (d->wait_event)
		CloseHandle(d->wait_event);

	delete d;
	_ring = {};
	logger::log(ring_channel, "upload ring destroyed");
}
} // namespace mars::graphics::dx
