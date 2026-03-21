#include "vk_internal.hpp"

#include <mars/graphics/backend/depth_buffer.hpp>
#include <mars/graphics/backend/framebuffer.hpp>
#include <mars/graphics/backend/vk/vk_render_pass.hpp>

namespace mars::graphics::vk {
namespace {
constexpr VkImageLayout kDepthSampledLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

void transition_image(
	VkCommandBuffer command_buffer,
	VkImage image,
	VkImageAspectFlags aspect_mask,
	VkImageLayout old_layout,
	VkImageLayout new_layout,
	VkAccessFlags src_access,
	VkAccessFlags dst_access,
	VkPipelineStageFlags src_stage,
	VkPipelineStageFlags dst_stage
) {
	if (old_layout == new_layout && src_access == dst_access)
		return;

	VkImageSubresourceRange subresource_range = {};
	subresource_range.aspectMask = aspect_mask;
	subresource_range.levelCount = 1u;
	subresource_range.layerCount = 1u;
	vk_cmd_pipeline_image_barrier2(
		command_buffer,
		src_stage,
		src_access,
		dst_stage,
		dst_access,
		old_layout,
		new_layout,
		image,
		subresource_range
	);
}
} // namespace

render_pass vk_render_pass_impl::vk_render_pass_create(const device& _device, const render_pass_create_params& _params) {
	auto* data = new vk_render_pass_data();
	data->format = _params.format;
	data->depth_format = _params.depth_format;
	data->load_operation = _params.load_operation;
	data->depth_clear_value = _params.depth_clear_value;
	data->actual_color_format = vk_format_from_mars(_params.format);

	render_pass result;
	result.engine = _device.engine;
	result.data.store(data);
	return result;
}

void vk_render_pass_impl::vk_render_pass_bind(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const depth_buffer* _depth_buffer, const render_pass_bind_param& _params) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* framebuffer_data = _framebuffer.data.expect<vk_framebuffer_data>();
	auto* render_pass_data = _render_pass.data.expect<vk_render_pass_data>();
	auto* depth_data = _depth_buffer != nullptr ? _depth_buffer->data.expect<vk_depth_buffer_data>() : nullptr;

	command_buffer_data->last_bound_framebuffer = framebuffer_data;
	command_buffer_data->last_bound_depth_buffer = depth_data;
	command_buffer_data->swapchain = framebuffer_data->is_swapchain ? framebuffer_data->swapchain_owner : nullptr;

	vk_cmd_pipeline_memory_barrier2(
		command_buffer_data->command_buffer,
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
			VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_EXT |
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
			VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_EXT |
			VK_ACCESS_SHADER_READ_BIT |
			VK_ACCESS_MEMORY_READ_BIT
	);
	command_buffer_data->has_global_memory_barrier = true;

	if (framebuffer_data->is_swapchain) {
		const VkImageLayout old_layout = framebuffer_data->swapchain_owner->image_layouts[framebuffer_data->swapchain_image_index];
		transition_image(
			command_buffer_data->command_buffer,
			framebuffer_data->color_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			old_layout,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			0u,
			vk_attachment_access_flags(),
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			vk_attachment_stage_flags()
		);
		framebuffer_data->swapchain_owner->image_layouts[framebuffer_data->swapchain_image_index] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	} else if (framebuffer_data->color_texture) {
		transition_image(
			command_buffer_data->command_buffer,
			framebuffer_data->color_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			framebuffer_data->color_texture->current_layout,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			framebuffer_data->color_texture->current_access,
			vk_attachment_access_flags(),
			framebuffer_data->color_texture->current_stage,
			vk_attachment_stage_flags()
		);
		framebuffer_data->color_texture->current_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		framebuffer_data->color_texture->current_access = vk_attachment_access_flags();
		framebuffer_data->color_texture->current_stage = vk_attachment_stage_flags();
	}

	if (depth_data != nullptr && depth_data->sampled) {
		transition_image(
			command_buffer_data->command_buffer,
			depth_data->image,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			depth_data->current_layout,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			depth_data->current_access,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			depth_data->current_stage,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
		);
		depth_data->current_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_data->current_access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		depth_data->current_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}

	VkClearValue clear_value = {};
	clear_value.color.float32[0] = _params.clear_color.x;
	clear_value.color.float32[1] = _params.clear_color.y;
	clear_value.color.float32[2] = _params.clear_color.z;
	clear_value.color.float32[3] = _params.clear_color.w;

	VkRenderingAttachmentInfo color_attachment = {};
	color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	color_attachment.imageView = framebuffer_data->color_view;
	color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachment.loadOp = render_pass_data->load_operation == MARS_RENDER_PASS_LOAD_OP_LOAD ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.clearValue = clear_value;

	VkClearValue depth_clear = {};
	depth_clear.depthStencil.depth = _params.clear_depth;

	VkRenderingAttachmentInfo depth_attachment = {};
	depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depth_attachment.imageView = depth_data != nullptr ? depth_data->dsv_view : VK_NULL_HANDLE;
	depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.clearValue = depth_clear;

	VkRenderingInfo rendering_info = {};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.renderArea.extent.width = static_cast<uint32_t>(_framebuffer.extent.x);
	rendering_info.renderArea.extent.height = static_cast<uint32_t>(_framebuffer.extent.y);
	rendering_info.layerCount = 1u;
	rendering_info.colorAttachmentCount = 1u;
	rendering_info.pColorAttachments = &color_attachment;
	rendering_info.pDepthAttachment = depth_data != nullptr ? &depth_attachment : nullptr;

	vkCmdBeginRendering(command_buffer_data->command_buffer, &rendering_info);
}

void vk_render_pass_impl::vk_render_pass_unbind(const render_pass&, const command_buffer& _command_buffer) {
	auto* command_buffer_data = _command_buffer.data.expect<vk_command_buffer_data>();
	auto* framebuffer_data = command_buffer_data->last_bound_framebuffer;
	auto* depth_data = command_buffer_data->last_bound_depth_buffer;
	if (!framebuffer_data)
		return;

	vkCmdEndRendering(command_buffer_data->command_buffer);

	if (framebuffer_data->is_swapchain) {
		transition_image(
			command_buffer_data->command_buffer,
			framebuffer_data->color_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			vk_attachment_access_flags(),
			0u,
			vk_attachment_stage_flags(),
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
		);
		framebuffer_data->swapchain_owner->image_layouts[framebuffer_data->swapchain_image_index] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	} else if (framebuffer_data->color_texture) {
		transition_image(
			command_buffer_data->command_buffer,
			framebuffer_data->color_image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			vk_attachment_access_flags(),
			VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
			vk_attachment_stage_flags(),
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		);
		framebuffer_data->color_texture->current_layout = VK_IMAGE_LAYOUT_GENERAL;
		framebuffer_data->color_texture->current_access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		framebuffer_data->color_texture->current_stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	if (depth_data != nullptr && depth_data->sampled) {
		transition_image(
			command_buffer_data->command_buffer,
			depth_data->image,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			kDepthSampledLayout,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
		);
		depth_data->current_layout = kDepthSampledLayout;
		depth_data->current_access = VK_ACCESS_SHADER_READ_BIT;
		depth_data->current_stage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	command_buffer_data->last_bound_framebuffer = nullptr;
	command_buffer_data->last_bound_depth_buffer = nullptr;
}

void vk_render_pass_impl::vk_render_pass_destroy(render_pass& _render_pass, const device&) {
	auto* data = _render_pass.data.expect<vk_render_pass_data>();
	delete data;
	_render_pass = {};
}
} // namespace mars::graphics::vk
