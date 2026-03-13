#include "vk_internal.hpp"

#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/backend/vk/vk_framebuffer.hpp>

namespace mars::graphics::vk {
namespace {
void create_depth_resources(vk_device_data* device_data, vk_framebuffer_data* framebuffer_data, const vector2<size_t>& size, mars_format_type depth_format) {
	if (depth_format == MARS_FORMAT_UNDEFINED)
		return;

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(size.x);
	image_info.extent.height = static_cast<uint32_t>(size.y);
	image_info.extent.depth = 1u;
	image_info.mipLevels = 1u;
	image_info.arrayLayers = 1u;
	image_info.format = vk_format_from_mars(depth_format);
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_expect<vkCreateImage>(device_data->device, &image_info, nullptr, &framebuffer_data->depth_image);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(device_data->device, framebuffer_data->depth_image, &requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = vk_find_memory_type(device_data, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vk_expect<vkAllocateMemory>(device_data->device, &alloc_info, nullptr, &framebuffer_data->depth_memory);
	vk_expect<vkBindImageMemory>(device_data->device, framebuffer_data->depth_image, framebuffer_data->depth_memory, 0u);

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = framebuffer_data->depth_image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = image_info.format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.levelCount = 1u;
	view_info.subresourceRange.layerCount = 1u;
	vk_expect<vkCreateImageView>(device_data->device, &view_info, nullptr, &framebuffer_data->depth_view);
}

VkImageView create_color_view(vk_device_data* device_data, const vk_texture_data* texture_data) {
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = texture_data->image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = texture_data->format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.levelCount = 1u;
	view_info.subresourceRange.layerCount = 1u;

	VkImageView view = VK_NULL_HANDLE;
	vk_expect<vkCreateImageView>(device_data->device, &view_info, nullptr, &view);
	return view;
}
} // namespace

framebuffer vk_framebuffer_impl::vk_framebuffer_create(const device& _device, const framebuffer_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* texture_data = _params.view.data.expect<vk_texture_data>();
	auto* render_pass_data = _params.render_pass.data.expect<vk_render_pass_data>();

	auto* data = new vk_framebuffer_data();
	data->color_image = texture_data->image;
	data->color_texture = texture_data;
	data->color_view = create_color_view(device_data, texture_data);
	create_depth_resources(device_data, data, _params.size, render_pass_data->depth_format);

	framebuffer result;
	result.engine = _device.engine;
	result.data.store(data);
	result.extent = _params.size;
	return result;
}

std::vector<framebuffer> vk_framebuffer_impl::vk_framebuffer_create_from_swapchain(const device& _device, const swapchain& _swapchain, const render_pass& _render_pass) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* swapchain_data = _swapchain.data.expect<vk_swapchain_data>();
	auto* render_pass_data = _render_pass.data.expect<vk_render_pass_data>();

	render_pass_data->actual_color_format = swapchain_data->format;

	std::vector<framebuffer> result;
	result.reserve(_swapchain.swapchain_size);
	for (size_t index = 0u; index < _swapchain.swapchain_size; ++index) {
		auto* data = new vk_framebuffer_data();
		data->is_swapchain = true;
		data->swapchain_owner = swapchain_data;
		data->swapchain_image_index = static_cast<uint32_t>(index);
		data->color_image = swapchain_data->images[index];
		data->color_view = swapchain_data->views[index];
		create_depth_resources(device_data, data, _swapchain.extent, render_pass_data->depth_format);

		framebuffer framebuffer_handle;
		framebuffer_handle.engine = _device.engine;
		framebuffer_handle.data.store(data);
		framebuffer_handle.extent = _swapchain.extent;
		result.push_back(framebuffer_handle);
	}
	return result;
}

void vk_framebuffer_impl::vk_framebuffer_destroy(framebuffer& _framebuffer, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = _framebuffer.data.expect<vk_framebuffer_data>();

	if (!data->is_swapchain && data->color_view != VK_NULL_HANDLE)
		vkDestroyImageView(device_data->device, data->color_view, nullptr);
	if (data->depth_view != VK_NULL_HANDLE)
		vkDestroyImageView(device_data->device, data->depth_view, nullptr);
	if (data->depth_image != VK_NULL_HANDLE)
		vkDestroyImage(device_data->device, data->depth_image, nullptr);
	if (data->depth_memory != VK_NULL_HANDLE)
		vkFreeMemory(device_data->device, data->depth_memory, nullptr);

	delete data;
	_framebuffer = {};
}
} // namespace mars::graphics::vk
