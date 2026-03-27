#include <mars/graphics/backend/vk/vk_backend.hpp>

#include <mars/graphics/backend/vk/vk_acceleration_structure.hpp>
#include <mars/graphics/backend/vk/vk_append_buffer.hpp>
#include <mars/graphics/backend/vk/vk_buffer.hpp>
#include <mars/graphics/backend/vk/vk_command_pool.hpp>
#include <mars/graphics/backend/vk/vk_command_queue.hpp>
#include <mars/graphics/backend/vk/vk_compute_pipeline.hpp>
#include <mars/graphics/backend/vk/vk_descriptor.hpp>
#include <mars/graphics/backend/vk/vk_depth_buffer.hpp>
#include <mars/graphics/backend/vk/vk_device.hpp>
#include <mars/graphics/backend/vk/vk_framebuffer.hpp>
#include <mars/graphics/backend/vk/vk_indirect_executor.hpp>
#include <mars/graphics/backend/vk/vk_pipeline.hpp>
#include <mars/graphics/backend/vk/vk_ray_tracing_pipeline.hpp>
#include <mars/graphics/backend/vk/vk_readback_buffer.hpp>
#include <mars/graphics/backend/vk/vk_render_pass.hpp>
#include <mars/graphics/backend/vk/vk_shader.hpp>
#include <mars/graphics/backend/vk/vk_swapchain.hpp>
#include <mars/graphics/backend/vk/vk_texture.hpp>
#include <mars/graphics/backend/vk/vk_timeline_fence.hpp>
#include <mars/graphics/backend/vk/vk_upload_ring.hpp>
#include <mars/graphics/backend/vk/vk_window.hpp>

namespace mars::graphics {
graphics_backend_functions* vulkan_t::get_functions() {
	static graphics_backend_functions functions{
		.window = {
			.window_create = &vk::vk_window_impl::vk_window_create,
			.window_destroy = &vk::vk_window_impl::vk_window_destroy,
		},
		.device = {
			.device_create = &vk::vk_device_impl::vk_device_create,
			.device_submit = &vk::vk_device_impl::vk_device_submit,
			.device_flush = &vk::vk_device_impl::vk_device_flush,
			.device_supports_feature = &vk::vk_device_impl::vk_device_supports_feature,
			.device_destroy = &vk::vk_device_impl::vk_device_destroy,
			.device_submit_compute = &vk::vk_device_impl::vk_device_submit_compute,
		},
		.command_queue = {
			.command_queue_create = &vk::vk_command_queue_impl::vk_command_queue_create,
			.command_queue_destroy = &vk::vk_command_queue_impl::vk_command_queue_destroy,
		},
		.swapchain = {
			.swapchain_create = &vk::vk_swapchain_impl::vk_swapchain_create,
			.swapchain_present = &vk::vk_swapchain_impl::vk_swapchain_present,
			.swapchain_get_back_buffer_index = &vk::vk_swapchain_impl::vk_swapchain_get_back_buffer_index,
			.swapchain_resize = &vk::vk_swapchain_impl::vk_swapchain_resize,
			.swapchain_destroy = &vk::vk_swapchain_impl::vk_swapchain_destroy,
		},
		.shader = {
			.shader_create = &vk::vk_shader_impl::vk_shader_create,
			.shader_destroy = &vk::vk_shader_impl::vk_shader_destroy,
		},
		.pipeline = {
			.pipeline_create = &vk::vk_pipeline_impl::vk_pipeline_create,
			.pipeline_bind = &vk::vk_pipeline_impl::vk_pipeline_bind,
			.pipeline_destroy = &vk::vk_pipeline_impl::vk_pipeline_destroy,
		},
		.compute_pipeline = {
			.compute_pipeline_create = &vk::vk_compute_pipeline_impl::vk_compute_pipeline_create,
			.compute_pipeline_bind = &vk::vk_compute_pipeline_impl::vk_compute_pipeline_bind,
			.compute_pipeline_destroy = &vk::vk_compute_pipeline_impl::vk_compute_pipeline_destroy,
		},
		.command_pool = {
			.command_pool_create = &vk::vk_command_pool_impl::vk_command_pool_create,
			.command_buffer_create = &vk::vk_command_pool_impl::vk_command_buffer_create,
			.command_buffer_reset = &vk::vk_command_pool_impl::vk_command_buffer_reset,
			.command_buffer_record = &vk::vk_command_pool_impl::vk_command_buffer_record,
			.command_buffer_record_end = &vk::vk_command_pool_impl::vk_command_buffer_record_end,
			.command_buffer_draw = &vk::vk_command_pool_impl::vk_command_buffer_draw,
			.command_buffer_draw_indexed = &vk::vk_command_pool_impl::vk_command_buffer_draw_indexed,
			.command_buffer_dispatch = &vk::vk_command_pool_impl::vk_command_buffer_dispatch,
			.command_buffer_set_push_constants = &vk::vk_command_pool_impl::vk_command_buffer_set_push_constants,
			.command_buffer_set_compute_push_constants = &vk::vk_command_pool_impl::vk_command_buffer_set_compute_push_constants,
			.command_buffer_memory_barrier = &vk::vk_command_pool_impl::vk_command_buffer_memory_barrier,
			.command_pool_destroy = &vk::vk_command_pool_impl::vk_command_pool_destroy,
			.command_buffer_begin_event = &vk::vk_command_pool_impl::vk_command_buffer_begin_event,
			.command_buffer_end_event = &vk::vk_command_pool_impl::vk_command_buffer_end_event,
			.command_buffer_trace_rays = &vk::vk_command_pool_impl::vk_command_buffer_trace_rays,
			.compute_command_pool_create = &vk::vk_command_pool_impl::vk_compute_command_pool_create,
		},
		.buffer = {
			.buffer_create = &vk::vk_buffer_impl::vk_buffer_create,
			.buffer_bind = &vk::vk_buffer_impl::vk_buffer_bind,
			.buffer_bind_index = &vk::vk_buffer_impl::vk_buffer_bind_index,
			.buffer_copy = &vk::vk_buffer_impl::vk_buffer_copy,
			.buffer_transition = &vk::vk_buffer_impl::vk_buffer_transition,
			.buffer_map = &vk::vk_buffer_impl::vk_buffer_map,
			.buffer_unmap = &vk::vk_buffer_impl::vk_buffer_unmap,
			.buffer_destroy = &vk::vk_buffer_impl::vk_buffer_destroy,
			.buffer_get_uav_index = &vk::vk_buffer_impl::vk_buffer_get_uav_index,
			.buffer_get_srv_index = &vk::vk_buffer_impl::vk_buffer_get_srv_index,
			.buffer_get_device_address = &vk::vk_buffer_impl::vk_buffer_get_device_address,
		},
		.texture = {
			.texture_create = &vk::vk_texture_impl::vk_texture_create,
			.texture_copy = &vk::vk_texture_impl::vk_texture_copy,
			.texture_transition = &vk::vk_texture_impl::vk_texture_transition,
			.texture_map = &vk::vk_texture_impl::vk_texture_map,
			.texture_unmap = &vk::vk_texture_impl::vk_texture_unmap,
			.texture_get_upload_layout = &vk::vk_texture_impl::vk_texture_get_upload_layout,
			.texture_get_srv_index = &vk::vk_texture_impl::vk_texture_get_srv_index,
			.texture_get_uav_base = &vk::vk_texture_impl::vk_texture_get_uav_base,
			.texture_destroy = &vk::vk_texture_impl::vk_texture_destroy,
		},
		.depth_buffer = {
			.depth_buffer_create = &vk::vk_depth_buffer_impl::vk_depth_buffer_create,
			.depth_buffer_transition = &vk::vk_depth_buffer_impl::vk_depth_buffer_transition,
			.depth_buffer_get_srv_index = &vk::vk_depth_buffer_impl::vk_depth_buffer_get_srv_index,
			.depth_buffer_destroy = &vk::vk_depth_buffer_impl::vk_depth_buffer_destroy,
		},
		.render_pass = {
			.render_pass_create = &vk::vk_render_pass_impl::vk_render_pass_create,
			.render_pass_bind = &vk::vk_render_pass_impl::vk_render_pass_bind,
			.render_pass_unbind = &vk::vk_render_pass_impl::vk_render_pass_unbind,
			.render_pass_destroy = &vk::vk_render_pass_impl::vk_render_pass_destroy,
		},
		.framebuffer = {
			.framebuffer_create = &vk::vk_framebuffer_impl::vk_framebuffer_create,
			.framebuffer_create_from_swapchain = &vk::vk_framebuffer_impl::vk_framebuffer_create_from_swapchain,
			.framebuffer_destroy = &vk::vk_framebuffer_impl::vk_framebuffer_destroy,
		},
		.descriptor = {
			.descriptor_create = &vk::vk_descriptor_impl::vk_descriptor_create,
			.descriptor_set_create = &vk::vk_descriptor_impl::vk_descriptor_set_create,
			.descriptor_set_create_compute = &vk::vk_descriptor_impl::vk_descriptor_set_create_compute,
			.descriptor_set_create_rt = &vk::vk_descriptor_impl::vk_descriptor_set_create_rt,
			.descriptor_set_bind = &vk::vk_descriptor_impl::vk_descriptor_set_bind,
			.descriptor_set_bind_compute = &vk::vk_descriptor_impl::vk_descriptor_set_bind_compute,
			.descriptor_set_bind_rt = &vk::vk_descriptor_impl::vk_descriptor_set_bind_rt,
			.descriptor_set_update_cbv = &vk::vk_descriptor_impl::vk_descriptor_set_update_cbv,
			.descriptor_destroy = &vk::vk_descriptor_impl::vk_descriptor_destroy,
		},
		.timeline_fence = {
			.timeline_fence_create = &vk::vk_timeline_fence_impl::create,
			.timeline_fence_alloc_value = &vk::vk_timeline_fence_impl::alloc_value,
			.timeline_fence_signal_on_queue = &vk::vk_timeline_fence_impl::signal_on_queue,
			.timeline_fence_wait_on_queue = &vk::vk_timeline_fence_impl::wait_on_queue,
			.timeline_fence_cpu_wait = &vk::vk_timeline_fence_impl::cpu_wait,
			.timeline_fence_completed_value = &vk::vk_timeline_fence_impl::completed_value,
			.timeline_fence_destroy = &vk::vk_timeline_fence_impl::destroy,
		},
		.upload_ring = {
			.upload_ring_create = &vk::vk_upload_ring_impl::create,
			.upload_ring_schedule = &vk::vk_upload_ring_impl::schedule,
			.upload_ring_flush = &vk::vk_upload_ring_impl::flush,
			.upload_ring_cpu_wait = &vk::vk_upload_ring_impl::cpu_wait,
			.upload_ring_destroy = &vk::vk_upload_ring_impl::destroy,
		},
		.readback_buffer = {
			.readback_buffer_create = &vk::vk_readback_buffer_impl::create,
			.readback_buffer_schedule = &vk::vk_readback_buffer_impl::schedule,
			.readback_buffer_flush = &vk::vk_readback_buffer_impl::flush,
			.readback_buffer_try_read = &vk::vk_readback_buffer_impl::try_read,
			.readback_buffer_release_slot = &vk::vk_readback_buffer_impl::release_slot,
			.readback_buffer_destroy = &vk::vk_readback_buffer_impl::destroy,
		},
		.indirect_executor = {
			.indirect_executor_create = &vk::vk_indirect_executor_impl::create,
			.indirect_executor_create_with_constant = &vk::vk_indirect_executor_impl::create_with_constant,
			.indirect_executor_create_with_constant_compute = &vk::vk_indirect_executor_impl::create_with_constant_compute,
			.indirect_executor_record_dispatch = &vk::vk_indirect_executor_impl::record_dispatch,
			.indirect_executor_record_draw_indexed = &vk::vk_indirect_executor_impl::record_draw_indexed,
			.indirect_executor_destroy = &vk::vk_indirect_executor_impl::destroy,
		},
		.append_buffer = {
			.append_buffer_create = &vk::vk_append_buffer_impl::create,
			.append_buffer_reset_counter = &vk::vk_append_buffer_impl::reset_counter,
			.append_buffer_get_counter_buffer = &vk::vk_append_buffer_impl::get_counter_buffer,
			.append_buffer_get_data_buffer = &vk::vk_append_buffer_impl::get_data_buffer,
			.append_buffer_destroy = &vk::vk_append_buffer_impl::destroy,
		},
		.acceleration_structure = {
			.blas_create = &vk::vk_acceleration_structure_impl::vk_blas_create,
			.blas_update = &vk::vk_acceleration_structure_impl::vk_blas_update,
			.blas_destroy = &vk::vk_acceleration_structure_impl::vk_blas_destroy,
			.tlas_create = &vk::vk_acceleration_structure_impl::vk_tlas_create,
			.tlas_build = &vk::vk_acceleration_structure_impl::vk_tlas_build,
			.tlas_get_srv_index = &vk::vk_acceleration_structure_impl::vk_tlas_get_srv_index,
			.tlas_destroy = &vk::vk_acceleration_structure_impl::vk_tlas_destroy,
		},
		.ray_tracing_pipeline = {
			.ray_tracing_pipeline_create = &vk::vk_ray_tracing_pipeline_impl::vk_ray_tracing_pipeline_create,
			.ray_tracing_pipeline_bind = &vk::vk_ray_tracing_pipeline_impl::vk_ray_tracing_pipeline_bind,
			.ray_tracing_pipeline_destroy = &vk::vk_ray_tracing_pipeline_impl::vk_ray_tracing_pipeline_destroy,
			.rt_pipeline_write_shader_identifiers = &vk::vk_ray_tracing_pipeline_impl::vk_rt_pipeline_write_shader_identifiers,
		},
	};

	return &functions;
}
} // namespace mars::graphics
