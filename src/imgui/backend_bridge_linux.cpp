#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <mars/imgui/backend_bridge.hpp>

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/device.hpp>
#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/backend/texture.hpp>
#include <mars/graphics/backend/vk/vk_backend.hpp>
#include <mars/graphics/functional/window.hpp>

#include "../graphics/backend/vk/vk_internal.hpp"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace mars::imgui {
namespace {
enum class backend_kind {
	none,
	vulkan,
};

struct backend_state {
	backend_kind kind = backend_kind::none;
	bool initialized = false;
	const mars::device* device = nullptr;
	const mars::swapchain* swapchain = nullptr;
	VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
	struct preview_texture_entry {
		const void* texture_key = nullptr;
		VkImageView image_view = VK_NULL_HANDLE;
		VkImageLayout image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	};
	std::vector<preview_texture_entry> preview_textures;
};

backend_state g_state;

bool is_vulkan_backend(const mars::device& device) {
	return device.engine == mars::graphics::vulkan_t::get_functions();
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
		if (entry.descriptor_set != VK_NULL_HANDLE)
			ImGui_ImplVulkan_RemoveTexture(entry.descriptor_set);
	}
	g_state.preview_textures.clear();
}

VkDescriptorSet get_or_create_preview_texture(graphics::vk::vk_device_data* device_data, const void* texture_key, VkImageView image_view, VkImageLayout image_layout) {
	for (auto& entry : g_state.preview_textures) {
		if (entry.texture_key != texture_key)
			continue;

		if (entry.image_view == image_view && entry.image_layout == image_layout)
			return entry.descriptor_set;

		if (entry.descriptor_set != VK_NULL_HANDLE)
			ImGui_ImplVulkan_RemoveTexture(entry.descriptor_set);

		entry.image_view = image_view;
		entry.image_layout = image_layout;
		entry.descriptor_set = ImGui_ImplVulkan_AddTexture(
			graphics::vk::vk_get_sampler(device_data, graphics::vk::vk_sampler_kind::linear_clamp),
			image_view,
			image_layout
		);
		return entry.descriptor_set;
	}

	backend_state::preview_texture_entry& entry = g_state.preview_textures.emplace_back();
	entry.texture_key = texture_key;
	entry.image_view = image_view;
	entry.image_layout = image_layout;
	entry.descriptor_set = ImGui_ImplVulkan_AddTexture(
		graphics::vk::vk_get_sampler(device_data, graphics::vk::vk_sampler_kind::linear_clamp),
		image_view,
		image_layout
	);
	return entry.descriptor_set;
}
} // namespace

bool backend_supported(const mars::device& device) {
	return is_vulkan_backend(device);
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

	if (is_vulkan_backend(device)) {
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
	case backend_kind::vulkan:
		ImGui_ImplVulkan_NewFrame();
		break;
	case backend_kind::none:
    default:
		return;
	}

	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

ImTextureRef texture_ref(const mars::texture& texture) {
	if (!g_state.initialized || texture.engine == nullptr)
		return {};

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

void render_draw_data(const mars::command_buffer& command_buffer) {
	if (!g_state.initialized)
		return;

	switch (g_state.kind) {
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

    if (g_state.kind == backend_kind::vulkan) {
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
