#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <mars/imgui/backend_bridge.hpp>

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/dx12/dx_backend.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/backend/texture.hpp>
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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> owned_heap;
	ID3D12DescriptorHeap* heap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE cpu_start = {};
	D3D12_GPU_DESCRIPTOR_HANDLE gpu_start = {};
	UINT descriptor_size = 0;
	std::vector<UINT> free_indices;

	void create(graphics::dx::dx_device_data* _device_data, UINT _descriptor_count) {
		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heap_desc.NumDescriptors = _descriptor_count;
		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (FAILED(_device_data->device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&owned_heap))))
			throw std::runtime_error("mars::imgui: failed to create DX12 descriptor heap");
		heap = owned_heap.Get();
		cpu_start = heap->GetCPUDescriptorHandleForHeapStart();
		gpu_start = heap->GetGPUDescriptorHandleForHeapStart();
		descriptor_size = _device_data->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		free_indices.clear();
		free_indices.reserve(_descriptor_count);
		for (UINT i = _descriptor_count; i > 0; --i)
			free_indices.push_back(i - 1);
	}

	void alloc(D3D12_CPU_DESCRIPTOR_HANDLE* _cpu, D3D12_GPU_DESCRIPTOR_HANDLE* _gpu) {
		if (free_indices.empty())
			throw std::runtime_error("mars::imgui: descriptor allocator exhausted");
		const UINT idx = free_indices.back();
		free_indices.pop_back();
		_cpu->ptr = cpu_start.ptr + idx * descriptor_size;
		_gpu->ptr = gpu_start.ptr + idx * descriptor_size;
	}

	void free(D3D12_CPU_DESCRIPTOR_HANDLE _cpu, D3D12_GPU_DESCRIPTOR_HANDLE _gpu) {
		const UINT cpu_idx = static_cast<UINT>((_cpu.ptr - cpu_start.ptr) / descriptor_size);
		const UINT gpu_idx = static_cast<UINT>((_gpu.ptr - gpu_start.ptr) / descriptor_size);
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
	struct preview_texture_entry {
		const void* texture_key = nullptr;
		UINT dx12_srv_bindless_idx = UINT32_MAX;
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = {};
		D3D12_GPU_DESCRIPTOR_HANDLE dx12_gpu_handle = {};
		VkImageView image_view = VK_NULL_HANDLE;
		VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	};
	std::vector<preview_texture_entry> preview_textures;
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

void clear_preview_textures() {
	for (auto& entry : g_state.preview_textures) {
		if (entry.dx12_cpu_handle.ptr != 0 && entry.dx12_gpu_handle.ptr != 0)
			g_state.dx_descriptors.free(entry.dx12_cpu_handle, entry.dx12_gpu_handle);
		if (entry.descriptor_set != VK_NULL_HANDLE)
			ImGui_ImplVulkan_RemoveTexture(entry.descriptor_set);
	}
	g_state.preview_textures.clear();
}

ImTextureRef get_or_create_preview_texture(graphics::dx::dx_device_data* _device_data, const void* _texture_key, UINT _srv_bindless_idx) {
	if (g_state.dx_descriptors.heap == nullptr)
		return {};

	auto* texture_data = static_cast<graphics::dx::dx_texture_data*>(const_cast<void*>(_texture_key));
	if (texture_data == nullptr || texture_data->resource == nullptr)
		return {};

	auto recreate_srv = [&](backend_state::preview_texture_entry& _entry) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = texture_data->format;
		if (texture_data->texture_type == MARS_TEXTURE_TYPE_CUBE) {
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srv_desc.TextureCube.MostDetailedMip = 0;
			srv_desc.TextureCube.MipLevels = static_cast<UINT>(texture_data->mip_levels);
			srv_desc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		else if (texture_data->array_size > 1) {
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srv_desc.Texture2DArray.MostDetailedMip = 0;
			srv_desc.Texture2DArray.MipLevels = static_cast<UINT>(std::max<size_t>(1, texture_data->mip_levels));
			srv_desc.Texture2DArray.FirstArraySlice = 0;
			srv_desc.Texture2DArray.ArraySize = static_cast<UINT>(texture_data->array_size);
			srv_desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		}
		else {
			srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels = static_cast<UINT>(std::max<size_t>(1, texture_data->mip_levels));
			srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
		}

		_device_data->device->CreateShaderResourceView(texture_data->resource.Get(), &srv_desc, _entry.dx12_cpu_handle);
		_entry.dx12_srv_bindless_idx = _srv_bindless_idx;
	};

	for (auto& entry : g_state.preview_textures) {
		if (entry.texture_key != _texture_key)
			continue;

		if (entry.dx12_srv_bindless_idx != _srv_bindless_idx)
			recreate_srv(entry);

		return ImTextureRef(static_cast<ImTextureID>(entry.dx12_gpu_handle.ptr));
	}

	backend_state::preview_texture_entry& entry = g_state.preview_textures.emplace_back();
	entry.texture_key = _texture_key;
	entry.dx12_srv_bindless_idx = _srv_bindless_idx;
	g_state.dx_descriptors.alloc(&entry.dx12_cpu_handle, &entry.dx12_gpu_handle);
	recreate_srv(entry);
	return ImTextureRef(static_cast<ImTextureID>(entry.dx12_gpu_handle.ptr));
}

VkDescriptorSet get_or_create_preview_texture(graphics::vk::vk_device_data* _device_data, const void* _texture_key, VkImageView _image_view, VkImageLayout _image_layout) {
	for (auto& entry : g_state.preview_textures) {
		if (entry.texture_key != _texture_key)
			continue;

		if (entry.image_view == _image_view && entry.image_layout == _image_layout)
			return entry.descriptor_set;

		if (entry.descriptor_set != VK_NULL_HANDLE)
			ImGui_ImplVulkan_RemoveTexture(entry.descriptor_set);

		entry.image_view = _image_view;
		entry.image_layout = _image_layout;
		entry.descriptor_set = ImGui_ImplVulkan_AddTexture(
			graphics::vk::vk_get_sampler(_device_data, graphics::vk::vk_sampler_kind::linear_clamp),
			_image_view,
			_image_layout
		);
		return entry.descriptor_set;
	}

	backend_state::preview_texture_entry& entry = g_state.preview_textures.emplace_back();
	entry.texture_key = _texture_key;
	entry.image_view = _image_view;
	entry.image_layout = _image_layout;
	entry.descriptor_set = ImGui_ImplVulkan_AddTexture(
		graphics::vk::vk_get_sampler(_device_data, graphics::vk::vk_sampler_kind::linear_clamp),
		_image_view,
		_image_layout
	);
	return entry.descriptor_set;
}
} // namespace

bool backend_supported(const mars::device& device) {
	return is_dx12_backend(device) || is_vulkan_backend(device);
}

void initialize_backend(const mars::window& window, const mars::device& device, const mars::swapchain& swapchain, size_t frames_in_flight) {
	if (g_state.initialized)
		shutdown_backend();

	IMGUI_CHECKVERSION();
	if (ImGui::GetCurrentContext() == nullptr)
		ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	if (io.Fonts->Fonts.empty())
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

ImTextureRef texture_ref(const mars::texture& texture) {
	if (!g_state.initialized || texture.engine == nullptr)
		return {};

	switch (g_state.kind) {
	case backend_kind::dx12: {
		auto* device_data = require_dx_device_data(*g_state.device);
		auto* texture_data = texture.data.get<graphics::dx::dx_texture_data>();
		if (texture_data == nullptr || texture_data->srv_bindless_idx == UINT32_MAX)
			return {};
		const void* texture_key = texture.data.get<void>();
		return get_or_create_preview_texture(device_data, texture_key, texture_data->srv_bindless_idx);
	}
	case backend_kind::vulkan: {
		auto* device_data = require_vk_device_data(*g_state.device);
		auto* texture_data = texture.data.get<graphics::vk::vk_texture_data>();
		if (texture_data == nullptr || texture_data->srv_view == VK_NULL_HANDLE)
			return {};

		const VkImageLayout image_layout = texture_data->current_layout == VK_IMAGE_LAYOUT_UNDEFINED
			? VK_IMAGE_LAYOUT_GENERAL
			: texture_data->current_layout;
		const void* texture_key = texture.data.get<void>();

		const VkDescriptorSet descriptor_set = get_or_create_preview_texture(device_data, texture_key, texture_data->srv_view, image_layout);
		return ImTextureRef(static_cast<ImTextureID>(reinterpret_cast<uintptr_t>(descriptor_set)));
	}
	case backend_kind::none:
		return {};
	}

	return {};
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

	clear_preview_textures();

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
