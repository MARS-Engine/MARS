#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <mars/imgui/backend_bridge.hpp>

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/dx12/dx_backend.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/backend/vk/vk_backend.hpp>
#include <mars/graphics/functional/window.hpp>

#include "../graphics/backend/dx12/dx_bindless_allocator.hpp"
#include "../graphics/backend/dx12/dx_internal.hpp"
#include "../graphics/backend/vk/vk_internal.hpp"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace mars::imgui {
namespace {
enum class backend_kind {
	none,
	dx12,
	vulkan,
};

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
	backend_kind kind = backend_kind::none;
	bool initialized = false;
	const mars::device* device = nullptr;
	const mars::swapchain* swapchain = nullptr;
	descriptor_allocator dx_descriptors;
	VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
};

backend_state g_state;

bool is_dx12_backend(const mars::device& device) {
	return device.engine == mars::graphics::directx12_t::get_functions();
}

bool is_vulkan_backend(const mars::device& device) {
	return device.engine == mars::graphics::vulkan_t::get_functions();
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

graphics::vk::vk_device_data* require_vk_device_data(const mars::device& device) {
	if (!is_vulkan_backend(device))
		throw std::runtime_error("mars::imgui: unsupported graphics backend");
	auto* vk_device = device.data.get<graphics::vk::vk_device_data>();
	if (!vk_device)
		throw std::runtime_error("mars::imgui: missing Vulkan device data");
	return vk_device;
}

graphics::vk::vk_command_buffer_data* require_vk_command_buffer_data(const mars::command_buffer& command_buffer) {
	auto* vk_cmd = command_buffer.data.get<graphics::vk::vk_command_buffer_data>();
	if (!vk_cmd)
		throw std::runtime_error("mars::imgui: command buffer is not backed by Vulkan data");
	return vk_cmd;
}

graphics::vk::vk_swapchain_data* require_vk_swapchain_data(const mars::swapchain& swapchain) {
	auto* vk_swapchain = swapchain.data.get<graphics::vk::vk_swapchain_data>();
	if (!vk_swapchain)
		throw std::runtime_error("mars::imgui: swapchain is not backed by Vulkan data");
	return vk_swapchain;
}
} // namespace

bool backend_supported(const mars::device& device) {
	return is_dx12_backend(device) || is_vulkan_backend(device);
}

void initialize_backend(const mars::window& window, const mars::device& device, const mars::swapchain& swapchain, size_t frames_in_flight) {
	if (g_state.initialized)
		shutdown_backend();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	io.Fonts->AddFontDefault();
	ImGui::StyleColorsDark();

	if (is_dx12_backend(device)) {
		auto* dx_device_data = require_dx_device_data(device);
		auto* dx_queue_data = dx_device_data->command_queue_data.get<graphics::dx::dx_command_queue_data>();
		if (!dx_queue_data)
			throw std::runtime_error("mars::imgui: missing DX12 direct queue data");

		g_state.dx_descriptors.create(dx_device_data, 8);
		ImGui_ImplSDL3_InitForD3D(window.sdl_window);

		ImGui_ImplDX12_InitInfo init = {};
		init.Device = dx_device_data->device.Get();
		init.CommandQueue = dx_queue_data->cmd_queue.Get();
		init.NumFramesInFlight = static_cast<UINT>(frames_in_flight);
		init.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		init.SrvDescriptorHeap = g_state.dx_descriptors.heap;
		init.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpu, D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
			auto* state = static_cast<backend_state*>(info->UserData);
			state->dx_descriptors.alloc(cpu, gpu);
		};
		init.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
			auto* state = static_cast<backend_state*>(info->UserData);
			state->dx_descriptors.free(cpu, gpu);
		};
		init.UserData = &g_state;

		if (!ImGui_ImplDX12_Init(&init))
			throw std::runtime_error("mars::imgui: ImGui DX12 backend initialization failed");

		g_state.kind = backend_kind::dx12;
	} else if (is_vulkan_backend(device)) {
		auto* vk_device_data = require_vk_device_data(device);
		auto* vk_swapchain_data = require_vk_swapchain_data(swapchain);

		ImGui_ImplSDL3_InitForVulkan(window.sdl_window);

		VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 128u};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 128u;
		pool_info.poolSizeCount = 1u;
		pool_info.pPoolSizes = &pool_size;
		if (vkCreateDescriptorPool(vk_device_data->device, &pool_info, nullptr, &g_state.vk_descriptor_pool) != VK_SUCCESS)
			throw std::runtime_error("mars::imgui: failed to create Vulkan descriptor pool");

		VkFormat color_format = vk_swapchain_data->format;
		VkPipelineRenderingCreateInfo rendering_info = {};
		rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		rendering_info.colorAttachmentCount = 1u;
		rendering_info.pColorAttachmentFormats = &color_format;

		ImGui_ImplVulkan_InitInfo init = {};
		init.ApiVersion = VK_API_VERSION_1_3;
		init.Instance = vk_device_data->instance;
		init.PhysicalDevice = vk_device_data->physical_device;
		init.Device = vk_device_data->device;
		init.QueueFamily = vk_device_data->direct_queue.family_index;
		init.Queue = vk_device_data->direct_queue.queue;
		init.DescriptorPool = g_state.vk_descriptor_pool;
		init.MinImageCount = static_cast<uint32_t>((std::max)(swapchain.swapchain_size, size_t{2}));
		init.ImageCount = static_cast<uint32_t>(swapchain.swapchain_size);
		init.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init.UseDynamicRendering = true;
		init.PipelineRenderingCreateInfo = rendering_info;
		init.MinAllocationSize = 1024 * 1024;

		if (!ImGui_ImplVulkan_Init(&init))
			throw std::runtime_error("mars::imgui: ImGui Vulkan backend initialization failed");

		g_state.kind = backend_kind::vulkan;
	} else {
		throw std::runtime_error("mars::imgui: unsupported graphics backend");
	}

	g_state.initialized = true;
	g_state.device = &device;
	g_state.swapchain = &swapchain;
}

void process_sdl_event(const SDL_Event& event) {
	if (g_state.initialized && ImGui::GetCurrentContext() != nullptr)
		ImGui_ImplSDL3_ProcessEvent(&event);
}

void new_frame() {
	if (!g_state.initialized)
		return;

	switch (g_state.kind) {
	case backend_kind::dx12:
		ImGui_ImplDX12_NewFrame();
		break;
	case backend_kind::vulkan:
		ImGui_ImplVulkan_NewFrame();
		break;
	case backend_kind::none:
		return;
	}

	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void render_draw_data(const mars::command_buffer& command_buffer) {
	if (!g_state.initialized)
		return;

	switch (g_state.kind) {
	case backend_kind::dx12: {
		auto* dx_cmd = require_dx_command_buffer_data(command_buffer);
		if (!g_state.dx_descriptors.heap)
			throw std::runtime_error("mars::imgui: missing shader-visible descriptor heap");
		ID3D12DescriptorHeap* heaps[] = {g_state.dx_descriptors.heap};
		dx_cmd->cmd_list->SetDescriptorHeaps(1, heaps);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx_cmd->cmd_list.Get());
		break;
	}
	case backend_kind::vulkan: {
		auto* vk_cmd = require_vk_command_buffer_data(command_buffer);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vk_cmd->command_buffer);
		break;
	}
	case backend_kind::none:
		break;
	}
}

void shutdown_backend() {
	if (!g_state.initialized)
		return;

	if (g_state.kind == backend_kind::dx12)
		ImGui_ImplDX12_Shutdown();
	else if (g_state.kind == backend_kind::vulkan) {
		ImGui_ImplVulkan_Shutdown();
		if (g_state.vk_descriptor_pool != VK_NULL_HANDLE) {
			auto* device_data = require_vk_device_data(*g_state.device);
			vkDestroyDescriptorPool(device_data->device, g_state.vk_descriptor_pool, nullptr);
		}
	}

	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	g_state = {};
}
} // namespace mars::imgui
