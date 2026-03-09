#include <mars/imgui/backend_bridge.hpp>

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/dx12/dx_backend.hpp>
#include <mars/graphics/functional/window.hpp>

#include "../graphics/backend/dx12/dx_bindless_allocator.hpp"
#include "../graphics/backend/dx12/dx_internal.hpp"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_sdl3.h>

#include <stdexcept>
#include <vector>

namespace mars::imgui {
namespace {

struct descriptor_allocator {
	ID3D12DescriptorHeap* heap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_start = {};
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_start = {};
	UINT descriptor_size = 0;
	std::vector<UINT> free_indices;

	void create(graphics::dx::dx_device_data* device_data, UINT descriptor_count) {
		heap = device_data->bindless_heap.Get();
		cpu_start = heap->GetCPUDescriptorHandleForHeapStart();
		gpu_start = heap->GetGPUDescriptorHandleForHeapStart();
		descriptor_size = device_data->bindless_descriptor_size;
		free_indices.clear();
		free_indices.reserve(descriptor_count);
		for (UINT i = descriptor_count; i > 0; --i)
			free_indices.push_back(graphics::dx::dx_allocate_bindless_srv_slot(device_data));
	}

	void alloc(D3D12_CPU_DESCRIPTOR_HANDLE* cpu, D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
		if (free_indices.empty())
			throw std::runtime_error("mars::imgui: descriptor allocator exhausted");
		const UINT idx = free_indices.back();
		free_indices.pop_back();
		cpu->ptr = cpu_start.ptr + idx * descriptor_size;
		gpu->ptr = gpu_start.ptr + idx * descriptor_size;
	}

	void free(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
		const UINT cpu_idx = static_cast<UINT>((cpu.ptr - cpu_start.ptr) / descriptor_size);
		const UINT gpu_idx = static_cast<UINT>((gpu.ptr - gpu_start.ptr) / descriptor_size);
		if (cpu_idx == gpu_idx)
			free_indices.push_back(cpu_idx);
	}
};

struct backend_state {
	bool initialized = false;
	const mars::device* device = nullptr;
	descriptor_allocator descriptors;
};

backend_state g_state;

bool is_dx12_backend(const mars::device& device) {
	return device.engine == mars::graphics::directx_12_t::get_functions();
}

graphics::dx::dx_device_data* require_dx_device_data(const mars::device& device) {
	if (!is_dx12_backend(device))
		throw std::runtime_error("mars::imgui: unsupported graphics backend");
	auto* dx_device = device.data.get<graphics::dx::dx_device_data>();
	if (!dx_device)
		throw std::runtime_error("mars::imgui: missing DX12 device data");
	return dx_device;
}

graphics::dx::dx_command_buffer_data* require_dx_command_buffer_data(const mars::command_buffer& command_buffer) {
	auto* dx_cmd = command_buffer.data.get<graphics::dx::dx_command_buffer_data>();
	if (!dx_cmd)
		throw std::runtime_error("mars::imgui: command buffer is not backed by DX12 data");
	return dx_cmd;
}

} // namespace

bool backend_supported(const mars::device& device) {
	return is_dx12_backend(device);
}

void initialize_backend(const mars::window& window, const mars::device& device, size_t frames_in_flight) {
	if (g_state.initialized)
		shutdown_backend();

	auto* dx_device_data = require_dx_device_data(device);
	auto* dx_queue_data = dx_device_data->command_queue_data.get<graphics::dx::dx_command_queue_data>();
	if (!dx_queue_data)
		throw std::runtime_error("mars::imgui: missing DX12 direct queue data");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.Fonts->AddFontDefault();
	ImGui::StyleColorsDark();

	g_state.descriptors.create(dx_device_data, 8);

	ImGui_ImplSDL3_InitForD3D(window.sdl_window);

	ImGui_ImplDX12_InitInfo init = {};
	init.Device = dx_device_data->device.Get();
	init.CommandQueue = dx_queue_data->cmd_queue.Get();
	init.NumFramesInFlight = static_cast<UINT>(frames_in_flight);
	init.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	init.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	init.SrvDescriptorHeap = g_state.descriptors.heap;
	init.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpu, D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
		auto* state = static_cast<backend_state*>(info->UserData);
		state->descriptors.alloc(cpu, gpu);
	};
	init.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
		auto* state = static_cast<backend_state*>(info->UserData);
		state->descriptors.free(cpu, gpu);
	};
	init.UserData = &g_state;

	if (!ImGui_ImplDX12_Init(&init))
		throw std::runtime_error("mars::imgui: ImGui DX12 backend initialization failed");

	g_state.initialized = true;
	g_state.device = &device;
}

void process_sdl_event(const SDL_Event& event) {
	if (g_state.initialized && ImGui::GetCurrentContext() != nullptr)
		ImGui_ImplSDL3_ProcessEvent(&event);
}

void new_frame() {
	if (!g_state.initialized)
		return;
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void render_draw_data(const mars::command_buffer& command_buffer) {
	if (!g_state.initialized)
		return;
	auto* dx_cmd = require_dx_command_buffer_data(command_buffer);
	if (!g_state.descriptors.heap)
		throw std::runtime_error("mars::imgui: missing shader-visible descriptor heap");
	ID3D12DescriptorHeap* heaps[] = {g_state.descriptors.heap};
	dx_cmd->cmd_list->SetDescriptorHeaps(1, heaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx_cmd->cmd_list.Get());
}

void shutdown_backend() {
	if (!g_state.initialized)
		return;
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	g_state = {};
}

} // namespace mars::imgui
