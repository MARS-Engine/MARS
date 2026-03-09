#pragma once

#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/buffer.hpp>
#include <mars/graphics/backend/command_queue.hpp>
#include <mars/graphics/backend/format.hpp>
#include <mars/graphics/backend/indirect_executor.hpp>
#include <mars/graphics/backend/pipeline.hpp>
#include <mars/meta/type_erased.hpp>

#include <atomic>
#include <cassert>
#include <d3d12.h>
#include <deque>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <string>
#include <utility>
#include <vector>
#include <windows.h>
#include <wrl.h>

namespace mars::graphics::dx {
inline mars::log_channel& dx12_log_channel() {
	static mars::log_channel channel("dx12");
	return channel;
}

template <typename T>
inline T* dx_expect_backend_data(T* data, const char* function_name, const char* handle_name) {
	mars::logger::assert_(data != nullptr, dx12_log_channel(), "{} called with null {}", function_name, handle_name);
	return data;
}

struct dx_framebuffer_data;

struct dx_device_data {
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	meta::type_erased_ptr command_queue_data;
	meta::type_erased_ptr compute_queue_data;
	meta::type_erased_ptr copy_queue_data;

	static constexpr UINT BINDLESS_SRV_BASE = 0;
	static constexpr UINT BINDLESS_SRV_COUNT = 32768;
	static constexpr UINT BINDLESS_UAV_BASE = 32768;
	static constexpr UINT BINDLESS_UAV_COUNT = 32768;
	static constexpr UINT BINDLESS_TOTAL = 65536;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> bindless_heap;
	UINT bindless_descriptor_size = 0;
	UINT next_bindless_srv_idx = BINDLESS_SRV_BASE;
	UINT next_bindless_uav_idx = BINDLESS_UAV_BASE;
	std::vector<std::pair<UINT, UINT>> free_bindless_srv_ranges;
	std::vector<std::pair<UINT, UINT>> free_bindless_uav_ranges;
};

enum class dx_upload_ring_state {
	idle,
	recording,
};

struct dx_command_queue_data {
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmd_queue;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fence_value = 0;
	HANDLE fence_event = nullptr;
};

struct dx_timeline_fence_data {
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	HANDLE event = nullptr;
	std::atomic<UINT64> next_value{1};

	dx_timeline_fence_data() = default;
	dx_timeline_fence_data(const dx_timeline_fence_data&) = delete;
	dx_timeline_fence_data& operator=(const dx_timeline_fence_data&) = delete;
};

struct dx_upload_ring_data {
	Microsoft::WRL::ComPtr<ID3D12Resource> upload_buffer;
	uint8_t* mapped_ptr = nullptr;
	size_t capacity = 0;
	size_t write_head = 0;
	size_t batch_start = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;

	ID3D12CommandQueue* copy_queue = nullptr;

	HANDLE wait_event = nullptr;
	UINT64 next_fence_value = 1;
	dx_upload_ring_state state = dx_upload_ring_state::idle;

	struct PendingBatch {
		size_t region_start;
		size_t region_end;
		UINT64 fence_value;
	};
	std::deque<PendingBatch> in_flight;
};

struct dx_readback_buffer_data {
	Microsoft::WRL::ComPtr<ID3D12Resource> readback_resource;
	uint8_t* mapped_ptr = nullptr;
	size_t slot_size = 0;
	uint32_t num_slots = 0;
	uint32_t current_slot = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;

	ID3D12CommandQueue* direct_queue = nullptr;

	HANDLE wait_event = nullptr;
	UINT64 next_fence_value = 1;
	bool pending_work = false;

	struct PendingBatch {
		uint32_t slot_index;
		size_t data_size;
		UINT64 fence_value;
	};
	std::deque<PendingBatch> in_flight;
};

inline dx_command_queue_data* dx_get_queue(dx_device_data* dev, mars_command_queue_type type) {
	switch (type) {
	case MARS_COMMAND_QUEUE_DIRECT:
		return dev->command_queue_data.get<dx_command_queue_data>();
	case MARS_COMMAND_QUEUE_COMPUTE:
		return dev->compute_queue_data.get<dx_command_queue_data>();
	case MARS_COMMAND_QUEUE_COPY:
		return dev->copy_queue_data.get<dx_command_queue_data>();
	}
	return nullptr;
}

struct dx_swapchain_data {
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swap_chain;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> render_targets;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap;
	UINT rtv_descriptor_size = 0;
};

struct dx_shader_data {
	Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixel_shader;
	Microsoft::WRL::ComPtr<ID3DBlob> compute_shader;
	std::string vertex_shader_path;
	std::string pixel_shader_path;
	std::string compute_shader_path;
};

struct dx_root_param_entry {
	size_t root_index;
	mars_pipeline_descriptor_type type;
	size_t binding;
};

struct dx_pipeline_data {
	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state;
	std::vector<dx_root_param_entry> root_layout;
	size_t push_constants_root_index = SIZE_MAX;
	size_t push_constant_count = 0;
	bool has_push_constants = false;
};

struct dx_compute_pipeline_data {
	Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state;
	std::vector<dx_root_param_entry> root_layout;
	size_t push_constants_root_index = SIZE_MAX;
	size_t push_constant_count = 0;
	bool has_push_constants = false;
};

struct dx_command_pool_data {
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmd_allocator;
	bool submitted = false;
};

struct dx_command_buffer_data {
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list;
	dx_command_pool_data* pool = nullptr;
	dx_framebuffer_data* last_bound_framebuffer = nullptr;
	ID3D12DescriptorHeap* bindless_heap_raw = nullptr;
};

struct dx_buffer_data {
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_VERTEX_BUFFER_VIEW vb_view = {};
	size_t size = 0;
	D3D12_RESOURCE_STATES dx12_state = D3D12_RESOURCE_STATE_COMMON;
	UINT uav_bindless_idx = UINT32_MAX;
	UINT srv_bindless_idx = UINT32_MAX;
};

struct dx_indirect_executor_data {
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> command_signature;
	mars_command_signature_type type = MARS_COMMAND_SIGNATURE_DISPATCH;
};

struct dx_texture_data {
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	Microsoft::WRL::ComPtr<ID3D12Resource> upload_resource;
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	mars_texture_type texture_type = MARS_TEXTURE_TYPE_2D;
	size_t mip_levels = 1;
	size_t array_size = 1;
	UINT srv_bindless_idx = UINT32_MAX;
	UINT uav_bindless_base = UINT32_MAX;
	UINT uav_descriptor_count = 0;
};

struct dx_render_pass_data {
	mars_format_type format;
	mars_format_type depth_format = MARS_FORMAT_UNDEFINED;
	float depth_clear_value = 1.0f;
};

struct dx_framebuffer_data {
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> render_target;
	D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> depth_target;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap;
	bool is_swapchain = false;
	D3D12_RESOURCE_STATES before_render_state = D3D12_RESOURCE_STATE_COMMON;
};

struct dx_descriptor_data {};

struct dx_descriptor_set_data {
	std::vector<std::pair<size_t, D3D12_GPU_VIRTUAL_ADDRESS>> cbv_bindings;
	const std::vector<dx_root_param_entry>* root_layout = nullptr;
};

struct dx_append_buffer_data {
	mars::buffer data_buf;
	mars::buffer counter_buf;
	mars::buffer zero_buf;
};
} // namespace mars::graphics::dx
