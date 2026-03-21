#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_swapchain.hpp>
#include <mars/graphics/functional/window.hpp>

#include <SDL3/SDL_vulkan.h>

#include <algorithm>
#include <limits>
#include <vector>

namespace mars::graphics::vk {
namespace {

VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats) {
	for (const auto& format : formats)
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	for (const auto& format : formats)
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	for (const auto& format : formats)
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return format;
	return formats.empty() ? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} : formats.front();
}

VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& present_modes) {
	for (VkPresentModeKHR mode : present_modes)
		if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			return mode;
	for (VkPresentModeKHR mode : present_modes)
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;
	return VK_PRESENT_MODE_FIFO_KHR;
}

void destroy_swapchain_resources(vk_device_data* device_data, vk_swapchain_data* swapchain_data, bool destroy_surface) {
	for (VkSemaphore semaphore : swapchain_data->render_finished_semaphores)
		if (semaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(device_data->device, semaphore, nullptr);
	for (VkSemaphore semaphore : swapchain_data->image_available_semaphores)
		if (semaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(device_data->device, semaphore, nullptr);
	swapchain_data->render_finished_semaphores.clear();
	swapchain_data->image_available_semaphores.clear();
	swapchain_data->active_render_finished_semaphore = VK_NULL_HANDLE;
	swapchain_data->active_image_available_semaphore = VK_NULL_HANDLE;
	swapchain_data->sync_index = 0u;

	for (VkImageView view : swapchain_data->views)
		if (view != VK_NULL_HANDLE)
			vkDestroyImageView(device_data->device, view, nullptr);
	swapchain_data->views.clear();
	swapchain_data->images.clear();

	if (swapchain_data->swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(device_data->device, swapchain_data->swapchain, nullptr);
	swapchain_data->swapchain = VK_NULL_HANDLE;

	if (destroy_surface && swapchain_data->surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(device_data->instance, swapchain_data->surface, nullptr);
	if (destroy_surface)
		swapchain_data->surface = VK_NULL_HANDLE;
}

void create_swapchain_resources(const device& _device, const window& _window, vk_swapchain_data* swapchain_data, size_t buffer_count) {
	auto* device_data = _device.data.expect<vk_device_data>();

	VkBool32 present_supported = VK_FALSE;
	vk_expect<vkGetPhysicalDeviceSurfaceSupportKHR>(
		device_data->physical_device,
		device_data->direct_queue.family_index,
		swapchain_data->surface,
		&present_supported
	);
	if (present_supported == VK_FALSE)
		mars::logger::error(vk_log_channel(), "Selected Vulkan graphics queue family does not support present");

	VkSurfaceCapabilitiesKHR capabilities = {};
	vk_expect<vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(device_data->physical_device, swapchain_data->surface, &capabilities);

	uint32_t format_count = 0u;
	vk_expect<vkGetPhysicalDeviceSurfaceFormatsKHR>(device_data->physical_device, swapchain_data->surface, &format_count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vk_expect<vkGetPhysicalDeviceSurfaceFormatsKHR>(device_data->physical_device, swapchain_data->surface, &format_count, formats.data());

	uint32_t present_mode_count = 0u;
	vk_expect<vkGetPhysicalDeviceSurfacePresentModesKHR>(device_data->physical_device, swapchain_data->surface, &present_mode_count, nullptr);
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vk_expect<vkGetPhysicalDeviceSurfacePresentModesKHR>(device_data->physical_device, swapchain_data->surface, &present_mode_count, present_modes.data());

	const VkSurfaceFormatKHR surface_format = choose_surface_format(formats);
	const VkPresentModeKHR present_mode = choose_present_mode(present_modes);

	VkExtent2D extent = {};
	extent.width = static_cast<uint32_t>((std::max)(_window.size.x, size_t{1}));
	extent.height = static_cast<uint32_t>((std::max)(_window.size.y, size_t{1}));
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		extent = capabilities.currentExtent;

	uint32_t min_image_count = static_cast<uint32_t>((std::max)(buffer_count, size_t{2}));
	min_image_count = (std::max)(min_image_count, capabilities.minImageCount);
	if (capabilities.maxImageCount != 0u)
		min_image_count = std::min(min_image_count, capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = swapchain_data->surface;
	create_info.minImageCount = min_image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1u;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.preTransform = capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	vk_expect<vkCreateSwapchainKHR>(device_data->device, &create_info, nullptr, &swapchain_data->swapchain);

	uint32_t image_count = 0u;
	vk_expect<vkGetSwapchainImagesKHR>(device_data->device, swapchain_data->swapchain, &image_count, nullptr);
	swapchain_data->images.resize(image_count);
	vk_expect<vkGetSwapchainImagesKHR>(device_data->device, swapchain_data->swapchain, &image_count, swapchain_data->images.data());
	swapchain_data->views.resize(image_count);
	swapchain_data->image_layouts.assign(image_count, VK_IMAGE_LAYOUT_UNDEFINED);

	for (uint32_t index = 0u; index < image_count; ++index) {
		VkImageViewCreateInfo view_info = {};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = swapchain_data->images[index];
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = surface_format.format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.levelCount = 1u;
		view_info.subresourceRange.layerCount = 1u;
		vk_expect<vkCreateImageView>(device_data->device, &view_info, nullptr, &swapchain_data->views[index]);
	}

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	swapchain_data->image_available_semaphores.resize(image_count, VK_NULL_HANDLE);
	swapchain_data->render_finished_semaphores.resize(image_count, VK_NULL_HANDLE);
	for (uint32_t index = 0u; index < image_count; ++index) {
		vk_expect<vkCreateSemaphore>(device_data->device, &semaphore_info, nullptr, &swapchain_data->image_available_semaphores[index]);
		vk_expect<vkCreateSemaphore>(device_data->device, &semaphore_info, nullptr, &swapchain_data->render_finished_semaphores[index]);
	}

	swapchain_data->format = surface_format.format;
	swapchain_data->color_space = surface_format.colorSpace;
	swapchain_data->extent = extent;
	swapchain_data->active_image_available_semaphore = VK_NULL_HANDLE;
	swapchain_data->active_render_finished_semaphore = VK_NULL_HANDLE;
	swapchain_data->sync_index = 0u;
	swapchain_data->image_acquired = false;
}

} // namespace

swapchain vk_swapchain_impl::vk_swapchain_create(const device& _device, const window& _window, const swapchain_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_swapchain_data();
	data->device_data = device_data;

	if (!SDL_Vulkan_CreateSurface(_window.sdl_window, device_data->instance, nullptr, &data->surface))
		mars::logger::error(vk_log_channel(), "SDL_Vulkan_CreateSurface failed: {}", SDL_GetError());

	create_swapchain_resources(_device, _window, data, _params.buffer_count);

	swapchain result;
	result.engine = _device.engine;
	result.data.store(data);
	result.swapchain_size = data->images.size();
	result.format = data->format == VK_FORMAT_B8G8R8A8_SRGB ? MARS_FORMAT_BGRA8_SRGB : MARS_FORMAT_RGBA8_UNORM;
	result.extent = {data->extent.width, data->extent.height};
	return result;
}

void vk_swapchain_impl::vk_swapchain_present(const swapchain& _swapchain, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _swapchain.data.expect<vk_swapchain_data>();
	if (!data->image_acquired)
		return;

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1u;
	present_info.pWaitSemaphores = &data->active_render_finished_semaphore;
	present_info.swapchainCount = 1u;
	present_info.pSwapchains = &data->swapchain;
	present_info.pImageIndices = &data->acquired_image_index;

	vk_expect<vkQueuePresentKHR>(device_data->direct_queue.queue, &present_info);
	data->image_acquired = false;
	if (!data->image_available_semaphores.empty())
		data->sync_index = (data->sync_index + 1u) % data->image_available_semaphores.size();
}

size_t vk_swapchain_impl::vk_swapchain_get_back_buffer_index(const swapchain& _swapchain) {
	auto* data = _swapchain.data.expect<vk_swapchain_data>();
	if (!data->image_acquired) {
		if (data->image_available_semaphores.empty() || data->render_finished_semaphores.empty())
			return data->acquired_image_index;
		data->active_image_available_semaphore = data->image_available_semaphores[data->sync_index];
		data->active_render_finished_semaphore = data->render_finished_semaphores[data->sync_index];
		const VkResult result = vk_expect<vkAcquireNextImageKHR>(
			data->device_data->device,
			data->swapchain,
			UINT64_MAX,
			data->active_image_available_semaphore,
			VK_NULL_HANDLE,
			&data->acquired_image_index
		);
		if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
			data->image_acquired = true;
	}
	return data->acquired_image_index;
}

void vk_swapchain_impl::vk_swapchain_resize(swapchain& _swapchain, const device& _device, const vector2<size_t>& _size) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _swapchain.data.expect<vk_swapchain_data>();
	vk_expect<vkDeviceWaitIdle>(device_data->device);
	destroy_swapchain_resources(device_data, data, false);

	window temp_window = {};
	temp_window.size = _size;
	create_swapchain_resources(_device, temp_window, data, _swapchain.swapchain_size);
	_swapchain.extent = _size;
}

void vk_swapchain_impl::vk_swapchain_destroy(swapchain& _swapchain, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _swapchain.data.expect<vk_swapchain_data>();
	destroy_swapchain_resources(device_data, data, true);
	delete data;
	_swapchain = {};
}
} // namespace mars::graphics::vk
