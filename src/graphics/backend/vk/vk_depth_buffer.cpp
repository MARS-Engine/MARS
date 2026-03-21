#include "vk_internal.hpp"

#include <mars/graphics/backend/vk/vk_depth_buffer.hpp>

namespace mars::graphics::vk {
namespace {
constexpr VkImageLayout kDepthSampledLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

void update_depth_descriptor(vk_device_data* device_data, uint32_t binding_index, VkImageView image_view) {
	VkDescriptorImageInfo image_info = {};
	image_info.imageView = image_view;
	image_info.imageLayout = kDepthSampledLayout;

	VkWriteDescriptorSet write = {};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.dstSet = device_data->bindless_set;
	write.dstBinding = 0u;
	write.dstArrayElement = binding_index;
	write.descriptorCount = 1u;
	write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	write.pImageInfo = &image_info;
	vkUpdateDescriptorSets(device_data->device, 1u, &write, 0u, nullptr);
}

void transition_depth_image(VkCommandBuffer command_buffer, vk_depth_buffer_data* depth_data, VkImageLayout new_layout, VkAccessFlags dst_access, VkPipelineStageFlags dst_stage) {
	if (depth_data == nullptr)
		return;

	if (depth_data->current_layout == new_layout &&
		depth_data->current_access == dst_access &&
		depth_data->current_stage == dst_stage)
		return;

	VkImageSubresourceRange subresource_range = {};
	subresource_range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	subresource_range.levelCount = 1u;
	subresource_range.layerCount = 1u;
	vk_cmd_pipeline_image_barrier2(command_buffer, depth_data->current_stage, depth_data->current_access, dst_stage, dst_access, depth_data->current_layout, new_layout, depth_data->image, subresource_range);
	depth_data->current_layout = new_layout;
	depth_data->current_access = dst_access;
	depth_data->current_stage = dst_stage;
}
} // namespace

depth_buffer vk_depth_buffer_impl::vk_depth_buffer_create(const device& _device, const depth_buffer_create_params& _params) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* data = new vk_depth_buffer_data();
	data->format = vk_depth_format_from_mars(_params.format);
	data->depth_format = _params.format;
	data->sampled = _params.sampled;

	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = static_cast<uint32_t>(_params.size.x);
	image_info.extent.height = static_cast<uint32_t>(_params.size.y);
	image_info.extent.depth = 1u;
	image_info.mipLevels = 1u;
	image_info.arrayLayers = 1u;
	image_info.format = data->format;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (_params.sampled)
		image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vk_expect<vkCreateImage>(device_data->device, &image_info, nullptr, &data->image);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(device_data->device, data->image, &requirements);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = requirements.size;
	alloc_info.memoryTypeIndex = vk_find_memory_type(device_data, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vk_expect<vkAllocateMemory>(device_data->device, &alloc_info, nullptr, &data->memory);
	vk_expect<vkBindImageMemory>(device_data->device, data->image, data->memory, 0u);

	VkImageViewCreateInfo dsv_view_info = {};
	dsv_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	dsv_view_info.image = data->image;
	dsv_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	dsv_view_info.format = data->format;
	dsv_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	dsv_view_info.subresourceRange.levelCount = 1u;
	dsv_view_info.subresourceRange.layerCount = 1u;
	vk_expect<vkCreateImageView>(device_data->device, &dsv_view_info, nullptr, &data->dsv_view);

	if (_params.sampled) {
		// Both attachment and sampled reads use the depth aspect only, so one
		// view is sufficient for both the DSV-style and SRV-style access paths.
		data->srv_view = data->dsv_view;
		data->srv_bindless_idx = vk_allocate_bindless_srv_slot(device_data);
		update_depth_descriptor(device_data, data->srv_bindless_idx, data->srv_view);
	}

	depth_buffer result;
	result.engine = _device.engine;
	result.data.store(data);
	result.size = _params.size;
	result.format = _params.format;
	return result;
}

void vk_depth_buffer_impl::vk_depth_buffer_transition(const command_buffer& _command_buffer, depth_buffer& _depth_buffer, mars_texture_state, mars_texture_state _after) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* depth_data = _depth_buffer.data.expect<vk_depth_buffer_data>();

	switch (_after) {
	case MARS_TEXTURE_STATE_SHADER_READ:
		transition_depth_image(
			command_buffer_data->command_buffer,
			depth_data,
			kDepthSampledLayout,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		);
		break;
	case MARS_TEXTURE_STATE_COMMON:
	default:
		transition_depth_image(command_buffer_data->command_buffer, depth_data, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		break;
	}
}

uint32_t vk_depth_buffer_impl::vk_depth_buffer_get_srv_index(const depth_buffer& _depth_buffer) {
	return _depth_buffer.data.expect<vk_depth_buffer_data>()->srv_bindless_idx;
}

void vk_depth_buffer_impl::vk_depth_buffer_destroy(depth_buffer& _depth_buffer, const device& _device) {
	auto* device_data = _device.data.expect<vk_device_data>();
	auto* depth_data = _depth_buffer.data.expect<vk_depth_buffer_data>();
	if (depth_data->srv_bindless_idx != std::numeric_limits<uint32_t>::max())
		vk_release_bindless_srv_slot(device_data, depth_data->srv_bindless_idx);
	if (depth_data->srv_view != VK_NULL_HANDLE && depth_data->srv_view != depth_data->dsv_view)
		vkDestroyImageView(device_data->device, depth_data->srv_view, nullptr);
	if (depth_data->dsv_view != VK_NULL_HANDLE)
		vkDestroyImageView(device_data->device, depth_data->dsv_view, nullptr);
	if (depth_data->image != VK_NULL_HANDLE)
		vkDestroyImage(device_data->device, depth_data->image, nullptr);
	if (depth_data->memory != VK_NULL_HANDLE)
		vkFreeMemory(device_data->device, depth_data->memory, nullptr);
	delete depth_data;
	_depth_buffer = {};
}
} // namespace mars::graphics::vk
