#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_texture.hpp>

#include <algorithm>

namespace mars::graphics::vk {
namespace {
uint32_t texture_layer_count(const vk_texture_data* data) {
	return data->texture_type == MARS_TEXTURE_TYPE_CUBE
			   ? static_cast<uint32_t>(data->array_size * 6u)
			   : static_cast<uint32_t>(data->array_size);
}

VkImageViewType sampled_view_type(const vk_texture_data* data) {
	if (data->texture_type == MARS_TEXTURE_TYPE_CUBE)
		return data->array_size > 1u ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
	return data->array_size > 1u ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
}

VkImageViewType storage_view_type(const vk_texture_data* data) {
	return texture_layer_count(data) > 1u ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
}

texture_upload_layout upload_layout_for(const vk_texture_data* data, const vector2<size_t>& size) {
	texture_upload_layout layout = {};
	layout.row_count = size.y;
	layout.row_size = size.x * vk_format_pixel_size(data->mars_format);
	layout.row_pitch = vk_align_up(layout.row_size, size_t{256});
	layout.offset = 0u;
	return layout;
}

VkImageUsageFlags image_usage_from_params(const texture_create_params& params) {
	VkImageUsageFlags usage = 0u;
	if ((params.usage & MARS_TEXTURE_USAGE_TRANSFER_SRC) == MARS_TEXTURE_USAGE_TRANSFER_SRC)
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if ((params.usage & MARS_TEXTURE_USAGE_TRANSFER_DST) == MARS_TEXTURE_USAGE_TRANSFER_DST)
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if ((params.usage & MARS_TEXTURE_USAGE_SAMPLED) == MARS_TEXTURE_USAGE_SAMPLED)
		usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if ((params.usage & MARS_TEXTURE_USAGE_COLOR_ATTACHMENT) == MARS_TEXTURE_USAGE_COLOR_ATTACHMENT)
		usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if ((params.usage & MARS_TEXTURE_USAGE_STORAGE) == MARS_TEXTURE_USAGE_STORAGE)
		usage |= VK_IMAGE_USAGE_STORAGE_BIT;
	return usage;
}

void update_image_descriptor(vk_device_data* device_data, VkDescriptorType descriptor_type, uint32_t binding_index, VkImageView image_view) {
	VkDescriptorImageInfo image_info = {};
	image_info.imageView = image_view;
	image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = device_data->bindless_set;
	write.dstBinding = 0u;
	write.dstArrayElement = binding_index;
	write.descriptorCount = 1u;
	write.descriptorType = descriptor_type;
	write.pImageInfo = &image_info;
	vkUpdateDescriptorSets(device_data->device, 1u, &write, 0u, nullptr);
}
} // namespace

texture vk_texture_impl::vk_texture_create(const device& _device, const texture_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_texture_data();
	data->format = vk_format_from_mars(_params.format);
	data->mars_format = _params.format;
	data->texture_type = _params.texture_type;
	data->mip_levels = _params.mip_levels;
	data->array_size = _params.array_size;

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(_params.size.x);
	image_info.extent.height = static_cast<uint32_t>(_params.size.y);
	image_info.extent.depth = 1u;
	image_info.mipLevels = static_cast<uint32_t>(_params.mip_levels);
	image_info.arrayLayers = _params.texture_type == MARS_TEXTURE_TYPE_CUBE
								 ? static_cast<uint32_t>(_params.array_size * 6u)
								 : static_cast<uint32_t>(_params.array_size);
	image_info.format = data->format;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = image_usage_from_params(_params);
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (_params.texture_type == MARS_TEXTURE_TYPE_CUBE)
		image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	vk_expect<vkCreateImage>(device_data->device, &image_info, nullptr, &data->image);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(device_data->device, data->image, &requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = vk_find_memory_type(device_data, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vk_expect<vkAllocateMemory>(device_data->device, &alloc_info, nullptr, &data->memory);
	vk_expect<vkBindImageMemory>(device_data->device, data->image, data->memory, 0u);

	VkImageViewCreateInfo srv_view_info = {};
	srv_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	srv_view_info.image = data->image;
	srv_view_info.viewType = sampled_view_type(data);
	srv_view_info.format = data->format;
	srv_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	srv_view_info.subresourceRange.levelCount = static_cast<uint32_t>(data->mip_levels);
	srv_view_info.subresourceRange.layerCount = texture_layer_count(data);
	vk_expect<vkCreateImageView>(device_data->device, &srv_view_info, nullptr, &data->srv_view);

	if ((_params.usage & MARS_TEXTURE_USAGE_TRANSFER_DST) == MARS_TEXTURE_USAGE_TRANSFER_DST) {
		const auto upload_layout = upload_layout_for(data, _params.size);
		data->upload_size = upload_layout.row_pitch * upload_layout.row_count;

		VkBufferCreateInfo upload_info = {};
		upload_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		upload_info.size = data->upload_size;
		upload_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		upload_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vk_expect<vkCreateBuffer>(device_data->device, &upload_info, nullptr, &data->upload_buffer);

		VkMemoryRequirements upload_requirements = {};
		vkGetBufferMemoryRequirements(device_data->device, data->upload_buffer, &upload_requirements);

		VkMemoryAllocateInfo upload_alloc = {};
		upload_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		upload_alloc.allocationSize = upload_requirements.size;
		upload_alloc.memoryTypeIndex = vk_find_memory_type(
			device_data,
			upload_requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);
		vk_expect<vkAllocateMemory>(device_data->device, &upload_alloc, nullptr, &data->upload_memory);
		vk_expect<vkBindBufferMemory>(device_data->device, data->upload_buffer, data->upload_memory, 0u);
	}

	if ((_params.usage & MARS_TEXTURE_USAGE_SAMPLED) == MARS_TEXTURE_USAGE_SAMPLED) {
		data->srv_bindless_idx = vk_allocate_bindless_srv_slot(device_data);
		update_image_descriptor(device_data, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, data->srv_bindless_idx, data->srv_view);
	}

	if ((_params.usage & MARS_TEXTURE_USAGE_STORAGE) == MARS_TEXTURE_USAGE_STORAGE) {
		const uint32_t total_slices = texture_layer_count(data);
		data->uav_descriptor_count = static_cast<uint32_t>(data->mip_levels) * total_slices;
		data->uav_bindless_base = vk_allocate_bindless_uav_range(device_data, data->uav_descriptor_count);
		data->uav_views.resize(data->uav_descriptor_count);

		for (uint32_t mip = 0u; mip < data->mip_levels; ++mip) {
			for (uint32_t slice = 0u; slice < total_slices; ++slice) {
				VkImageViewCreateInfo uav_view_info = {};
				uav_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				uav_view_info.image = data->image;
				uav_view_info.viewType = storage_view_type(data);
				uav_view_info.format = data->format;
				uav_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				uav_view_info.subresourceRange.baseMipLevel = mip;
				uav_view_info.subresourceRange.levelCount = 1u;
				uav_view_info.subresourceRange.baseArrayLayer = slice;
				uav_view_info.subresourceRange.layerCount = 1u;

				const uint32_t descriptor_index = mip * total_slices + slice;
				vk_expect<vkCreateImageView>(device_data->device, &uav_view_info, nullptr, &data->uav_views[descriptor_index]);
				update_image_descriptor(device_data, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, data->uav_bindless_base + descriptor_index, data->uav_views[descriptor_index]);
			}
		}
	}

	data->current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	data->current_access = 0u;
	data->current_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	texture result;
	result.engine = _device.engine;
	result.data.store(data);
	result.view.data.store(data);
	result.size = _params.size;
	result.channels = 4;
	result.format_size = _params.format == MARS_FORMAT_RGBA16_SFLOAT ? 2 : 1;
	return result;
}

void vk_texture_impl::vk_texture_copy(texture& _texture, buffer&, const command_buffer& _command_buffer, size_t _offset) {
	auto* texture_data = _texture.data.expect<vk_texture_data>();
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	if (texture_data->upload_buffer == VK_NULL_HANDLE)
		return;

	const auto copy_dst_state = vk_texture_barrier_state_from_mars(MARS_TEXTURE_STATE_COPY_DST);
	if (texture_data->current_layout != copy_dst_state.layout ||
		texture_data->current_access != copy_dst_state.access ||
		texture_data->current_stage != copy_dst_state.stage) {
		VkImageSubresourceRange subresource_range = {};
		subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource_range.levelCount = static_cast<uint32_t>(texture_data->mip_levels);
		subresource_range.layerCount = texture_layer_count(texture_data);
		vk_cmd_pipeline_image_barrier2(
			command_buffer_data->command_buffer,
			texture_data->current_stage,
			texture_data->current_access,
			copy_dst_state.stage,
			copy_dst_state.access,
			texture_data->current_layout,
			copy_dst_state.layout,
			texture_data->image,
			subresource_range
		);
		texture_data->current_layout = copy_dst_state.layout;
		texture_data->current_access = copy_dst_state.access;
		texture_data->current_stage = copy_dst_state.stage;
	}

	const texture_upload_layout layout = upload_layout_for(texture_data, _texture.size);
	const size_t pixel_size = (std::max)(vk_format_pixel_size(texture_data->mars_format), size_t{1});

	VkBufferImageCopy region = {};
	region.bufferOffset = _offset;
	region.bufferRowLength = static_cast<uint32_t>(layout.row_pitch / pixel_size);
	region.bufferImageHeight = static_cast<uint32_t>(layout.row_count);
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1u;
	region.imageExtent.width = static_cast<uint32_t>(_texture.size.x);
	region.imageExtent.height = static_cast<uint32_t>(_texture.size.y);
	region.imageExtent.depth = 1u;
	vkCmdCopyBufferToImage(command_buffer_data->command_buffer, texture_data->upload_buffer, texture_data->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &region);
}

void vk_texture_impl::vk_texture_transition(const command_buffer& _command_buffer, texture& _texture, mars_texture_state, mars_texture_state _after) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* texture_data = _texture.data.expect<vk_texture_data>();
	const auto target = vk_texture_barrier_state_from_mars(_after);
	if (texture_data->current_layout == target.layout && texture_data->current_access == target.access && texture_data->current_stage == target.stage)
		return;

	VkImageSubresourceRange subresource_range = {};
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.levelCount = static_cast<uint32_t>(texture_data->mip_levels);
	subresource_range.layerCount = texture_layer_count(texture_data);
	vk_cmd_pipeline_image_barrier2(
		command_buffer_data->command_buffer,
		texture_data->current_stage,
		texture_data->current_access,
		target.stage,
		target.access,
		texture_data->current_layout,
		target.layout,
		texture_data->image,
		subresource_range
	);

	texture_data->current_layout = target.layout;
	texture_data->current_access = target.access;
	texture_data->current_stage = target.stage;
}

void* vk_texture_impl::vk_texture_map(texture& _texture, const device& _device) {
	auto* texture_data = _texture.data.expect<vk_texture_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	if (texture_data->upload_buffer == VK_NULL_HANDLE)
		return nullptr;
	if (!texture_data->upload_mapped)
		vk_expect<vkMapMemory>(device_data->device, texture_data->upload_memory, 0u, texture_data->upload_size, 0u, &texture_data->upload_mapped);
	return texture_data->upload_mapped;
}

void vk_texture_impl::vk_texture_unmap(texture& _texture, const device& _device) {
	auto* texture_data = _texture.data.expect<vk_texture_data>();
	auto* device_data = _device.data.expect<vk_device_data>();
	if (texture_data->upload_mapped) {
		vkUnmapMemory(device_data->device, texture_data->upload_memory);
		texture_data->upload_mapped = nullptr;
	}
}

texture_upload_layout vk_texture_impl::vk_texture_get_upload_layout(texture& _texture, const device&) {
	auto* texture_data = _texture.data.expect<vk_texture_data>();
	texture_upload_layout layout = {};
	if (texture_data->upload_buffer == VK_NULL_HANDLE)
		return layout;
	return upload_layout_for(texture_data, _texture.size);
}

uint32_t vk_texture_impl::vk_texture_get_srv_index(const texture& _texture) {
	return _texture.data.expect<vk_texture_data>()->srv_bindless_idx;
}

uint32_t vk_texture_impl::vk_texture_get_uav_base(const texture& _texture) {
	return _texture.data.expect<vk_texture_data>()->uav_bindless_base;
}

void vk_texture_impl::vk_texture_destroy(texture& _texture, const device& _device) {
	auto* texture_data = _texture.data.expect<vk_texture_data>();
	auto* device_data = _device.data.expect<vk_device_data>();

	vk_release_bindless_srv_slot(device_data, texture_data->srv_bindless_idx);
	vk_release_bindless_uav_range(device_data, texture_data->uav_bindless_base, texture_data->uav_descriptor_count);

	for (VkImageView view : texture_data->uav_views)
		if (view != VK_NULL_HANDLE)
			vkDestroyImageView(device_data->device, view, nullptr);
	texture_data->uav_views.clear();

	if (texture_data->srv_view != VK_NULL_HANDLE)
		vkDestroyImageView(device_data->device, texture_data->srv_view, nullptr);
	if (texture_data->upload_mapped)
		vkUnmapMemory(device_data->device, texture_data->upload_memory);
	vk_destroy_buffer(device_data, texture_data->upload_buffer, texture_data->upload_memory);
	if (texture_data->image != VK_NULL_HANDLE)
		vkDestroyImage(device_data->device, texture_data->image, nullptr);
	if (texture_data->memory != VK_NULL_HANDLE)
		vkFreeMemory(device_data->device, texture_data->memory, nullptr);

	delete texture_data;
	_texture = {};
}
} // namespace mars::graphics::vk
