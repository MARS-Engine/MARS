#include "vk_internal.hpp"

#include <mars/graphics/backend/swapchain.hpp>
#include <mars/graphics/backend/vk/vk_framebuffer.hpp>

namespace mars::graphics::vk {
namespace {
VkImageView create_color_view(vk_device_data* _device_data, const vk_texture_data* _texture_data) {
	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = _texture_data->image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = _texture_data->format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.levelCount = 1u;
	view_info.subresourceRange.layerCount = 1u;

	VkImageView view = VK_NULL_HANDLE;
	vk_expect<vkCreateImageView>(_device_data->device, &view_info, nullptr, &view);
	return view;
}
} // namespace

framebuffer vk_framebuffer_impl::vk_framebuffer_create(const device& _device, const framebuffer_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	(void)_params.framebuffer_render_pass;

	auto* data = new vk_framebuffer_data();
	data->color_images.reserve(_params.views.size());
	data->color_views.reserve(_params.views.size());
	data->color_textures.reserve(_params.views.size());
	for (const auto& view : _params.views) {
		auto* texture_data = view.data.expect<vk_texture_data>();
		data->color_images.push_back(texture_data->image);
		data->color_textures.push_back(texture_data);
		data->color_views.push_back(create_color_view(device_data, texture_data));
	}

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

	render_pass_data->actual_color_formats = {swapchain_data->format};

	std::vector<framebuffer> result;
	result.reserve(_swapchain.swapchain_size);
	for (size_t index = 0u; index < _swapchain.swapchain_size; ++index) {
		auto* data = new vk_framebuffer_data();
		data->is_swapchain = true;
		data->swapchain_owner = swapchain_data;
		data->swapchain_image_index = static_cast<uint32_t>(index);
		data->color_images.push_back(swapchain_data->images[index]);
		data->color_views.push_back(swapchain_data->views[index]);
		data->color_textures.push_back(nullptr);

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

	if (!data->is_swapchain) {
		for (VkImageView view : data->color_views) {
			if (view != VK_NULL_HANDLE)
				vkDestroyImageView(device_data->device, view, nullptr);
		}
	}

	delete data;
	_framebuffer = {};
}
} // namespace mars::graphics::vk
