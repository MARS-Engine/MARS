#include <mars/graphics/backend/dx12/dx_backend.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>

#include <mars/graphics/backend/dx12/dx_append_buffer.hpp>
#include <mars/graphics/backend/dx12/dx_buffer.hpp>
#include <mars/graphics/backend/dx12/dx_command_pool.hpp>
#include <mars/graphics/backend/dx12/dx_command_queue.hpp>
#include <mars/graphics/backend/dx12/dx_compute_pipeline.hpp>
#include <mars/graphics/backend/dx12/dx_descriptor.hpp>
#include <mars/graphics/backend/dx12/dx_device.hpp>
#include <mars/graphics/backend/dx12/dx_framebuffer.hpp>
#include <mars/graphics/backend/dx12/dx_indirect_executor.hpp>
#include <mars/graphics/backend/dx12/dx_pipeline.hpp>
#include <mars/graphics/backend/dx12/dx_readback_buffer.hpp>
#include <mars/graphics/backend/dx12/dx_render_pass.hpp>
#include <mars/graphics/backend/dx12/dx_shader.hpp>
#include <mars/graphics/backend/dx12/dx_swapchain.hpp>
#include <mars/graphics/backend/dx12/dx_texture.hpp>
#include <mars/graphics/backend/dx12/dx_timeline_fence.hpp>
#include <mars/graphics/backend/dx12/dx_upload_ring.hpp>
#include <mars/graphics/backend/dx12/dx_window.hpp>

namespace mars {
namespace graphics {
graphics_backend_functions* directx_12_t::get_functions() {
	static graphics_backend_functions functions{
	    .window = {
		.window_create = &dx::dx_window_impl::dx_window_create,
		.window_destroy = &dx::dx_window_impl::dx_window_destroy,
	    },
	    .device = {
		.device_create = &dx::dx_device_impl::dx_device_create,
		.device_submit = &dx::dx_device_impl::dx_device_submit,
		.device_flush = &dx::dx_device_impl::dx_device_flush,
		.device_destroy = &dx::dx_device_impl::dx_device_destroy,
	    },
	    .command_queue = {
		.command_queue_create = &dx::dx_command_queue_impl::dx_command_queue_create,
		.command_queue_destroy = &dx::dx_command_queue_impl::dx_command_queue_destroy,
	    },
	    .swapchain = {
		.swapchain_create = &dx::dx_swapchain_impl::dx_swapchain_create,
		.swapchain_present = &dx::dx_swapchain_impl::dx_swapchain_present,
		.swapchain_get_back_buffer_index = &dx::dx_swapchain_impl::dx_swapchain_get_back_buffer_index,
		.swapchain_resize = &dx::dx_swapchain_impl::dx_swapchain_resize,
		.swapchain_destroy = &dx::dx_swapchain_impl::dx_swapchain_destroy,
	    },
	    .shader = {
		.shader_create = &dx::dx_shader_impl::dx_shader_create,
		.shader_destroy = &dx::dx_shader_impl::dx_shader_destroy,
	    },
	    .pipeline = {
		.pipeline_create = &dx::dx_pipeline_impl::dx_pipeline_create,
		.pipeline_bind = &dx::dx_pipeline_impl::dx_pipeline_bind,
		.pipeline_destroy = &dx::dx_pipeline_impl::dx_pipeline_destroy,
	    },
	    .compute_pipeline = {
		.compute_pipeline_create = &dx::dx_compute_pipeline_impl::dx_compute_pipeline_create,
		.compute_pipeline_bind = &dx::dx_compute_pipeline_impl::dx_compute_pipeline_bind,
		.compute_pipeline_destroy = &dx::dx_compute_pipeline_impl::dx_compute_pipeline_destroy,
	    },
	    .command_pool = {
		.command_pool_create = &dx::dx_command_pool_impl::dx_command_pool_create,
		.command_buffer_create = &dx::dx_command_pool_impl::dx_command_buffer_create,
		.command_buffer_reset = &dx::dx_command_pool_impl::dx_command_buffer_reset,
		.command_buffer_record = &dx::dx_command_pool_impl::dx_command_buffer_record,
		.command_buffer_record_end = &dx::dx_command_pool_impl::dx_command_buffer_record_end,
		.command_buffer_draw = &dx::dx_command_pool_impl::dx_command_buffer_draw,
		.command_buffer_draw_indexed = &dx::dx_command_pool_impl::dx_command_buffer_draw_indexed,
		.command_buffer_dispatch = &dx::dx_command_pool_impl::dx_command_buffer_dispatch,
		.command_buffer_set_push_constants = &dx::dx_command_pool_impl::dx_command_buffer_set_push_constants,
		.command_buffer_set_compute_push_constants = &dx::dx_command_pool_impl::dx_command_buffer_set_compute_push_constants,
		.command_buffer_memory_barrier = &dx::dx_command_pool_impl::dx_command_buffer_memory_barrier,
		.command_pool_destroy = &dx::dx_command_pool_impl::dx_command_pool_destroy,
		.command_buffer_begin_event = &dx::dx_command_pool_impl::dx_command_buffer_begin_event,
		.command_buffer_end_event = &dx::dx_command_pool_impl::dx_command_buffer_end_event,
	    },
	    .buffer = {
		.buffer_create = &dx::dx_buffer_impl::dx_buffer_create,
		.buffer_bind = &dx::dx_buffer_impl::dx_buffer_bind,
		.buffer_bind_index = &dx::dx_buffer_impl::dx_buffer_bind_index,
		.buffer_copy = &dx::dx_buffer_impl::dx_buffer_copy,
		.buffer_transition = &dx::dx_buffer_impl::dx_buffer_transition,
		.buffer_map = &dx::dx_buffer_impl::dx_buffer_map,
		.buffer_unmap = &dx::dx_buffer_impl::dx_buffer_unmap,
		.buffer_destroy = &dx::dx_buffer_impl::dx_buffer_destroy,
		.buffer_get_uav_index = &dx::dx_buffer_impl::dx_buffer_get_uav_index,
		.buffer_get_srv_index = &dx::dx_buffer_impl::dx_buffer_get_srv_index,
	    },
	    .texture = {
		.texture_create = &dx::dx_texture_impl::dx_texture_create,
		.texture_copy = &dx::dx_texture_impl::dx_texture_copy,
		.texture_transition = &dx::dx_texture_impl::dx_texture_transition,
		.texture_map = &dx::dx_texture_impl::dx_texture_map,
		.texture_unmap = &dx::dx_texture_impl::dx_texture_unmap,
		.texture_get_upload_layout = &dx::dx_texture_impl::dx_texture_get_upload_layout,
		.texture_get_srv_index = &dx::dx_texture_impl::dx_texture_get_srv_index,
		.texture_get_uav_base = &dx::dx_texture_impl::dx_texture_get_uav_base,
		.texture_destroy = &dx::dx_texture_impl::dx_texture_destroy,
	    },
	    .render_pass = {
		.render_pass_create = &dx::dx_render_pass_impl::dx_render_pass_create,
		.render_pass_bind = &dx::dx_render_pass_impl::dx_render_pass_bind,
		.render_pass_unbind = &dx::dx_render_pass_impl::dx_render_pass_unbind,
		.render_pass_destroy = &dx::dx_render_pass_impl::dx_render_pass_destroy,
	    },
	    .framebuffer = {
		.framebuffer_create = &dx::dx_framebuffer_impl::dx_framebuffer_create,
		.framebuffer_create_from_swapchain = &dx::dx_framebuffer_impl::dx_framebuffer_create_from_swapchain,
		.framebuffer_destroy = &dx::dx_framebuffer_impl::dx_framebuffer_destroy,
	    },
	    .descriptor = {
		.descriptor_create = &dx::dx_descriptor_impl::dx_descriptor_create,
		.descriptor_set_create = &dx::dx_descriptor_impl::dx_descriptor_set_create,
		.descriptor_set_create_compute = &dx::dx_descriptor_impl::dx_descriptor_set_create_compute,
		.descriptor_set_bind = &dx::dx_descriptor_impl::dx_descriptor_set_bind,
		.descriptor_set_bind_compute = &dx::dx_descriptor_impl::dx_descriptor_set_bind_compute,
		.descriptor_set_update_cbv = &dx::dx_descriptor_impl::dx_descriptor_set_update_cbv,
		.descriptor_destroy = &dx::dx_descriptor_impl::dx_descriptor_destroy,
	    },
	    .timeline_fence = {
		.timeline_fence_create = &dx::dx_timeline_fence_impl::create,
		.timeline_fence_alloc_value = &dx::dx_timeline_fence_impl::alloc_value,
		.timeline_fence_signal_on_queue = &dx::dx_timeline_fence_impl::signal_on_queue,
		.timeline_fence_wait_on_queue = &dx::dx_timeline_fence_impl::wait_on_queue,
		.timeline_fence_cpu_wait = &dx::dx_timeline_fence_impl::cpu_wait,
		.timeline_fence_completed_value = &dx::dx_timeline_fence_impl::completed_value,
		.timeline_fence_destroy = &dx::dx_timeline_fence_impl::destroy,
	    },
	    .upload_ring = {
		.upload_ring_create = &dx::dx_upload_ring_impl::create,
		.upload_ring_schedule = &dx::dx_upload_ring_impl::schedule,
		.upload_ring_flush = &dx::dx_upload_ring_impl::flush,
		.upload_ring_cpu_wait = &dx::dx_upload_ring_impl::cpu_wait,
		.upload_ring_destroy = &dx::dx_upload_ring_impl::destroy,
	    },
	    .readback_buffer = {
		.readback_buffer_create = &dx::dx_readback_buffer_impl::create,
		.readback_buffer_schedule = &dx::dx_readback_buffer_impl::schedule,
		.readback_buffer_flush = &dx::dx_readback_buffer_impl::flush,
		.readback_buffer_try_read = &dx::dx_readback_buffer_impl::try_read,
		.readback_buffer_release_slot = &dx::dx_readback_buffer_impl::release_slot,
		.readback_buffer_destroy = &dx::dx_readback_buffer_impl::destroy,
	    },
	    .indirect_executor = {
		.indirect_executor_create = &dx::dx_indirect_executor_impl::create,
		.indirect_executor_create_with_constant = &dx::dx_indirect_executor_impl::create_with_constant,
		.indirect_executor_create_with_constant_compute = &dx::dx_indirect_executor_impl::create_with_constant_compute,
		.indirect_executor_record_dispatch = &dx::dx_indirect_executor_impl::record_dispatch,
		.indirect_executor_record_draw_indexed = &dx::dx_indirect_executor_impl::record_draw_indexed,
		.indirect_executor_destroy = &dx::dx_indirect_executor_impl::destroy,
	    },
	    .append_buffer = {
		.append_buffer_create = &dx::dx_append_buffer_impl::create,
		.append_buffer_reset_counter = &dx::dx_append_buffer_impl::reset_counter,
		.append_buffer_get_counter_buffer = &dx::dx_append_buffer_impl::get_counter_buffer,
		.append_buffer_get_data_buffer = &dx::dx_append_buffer_impl::get_data_buffer,
		.append_buffer_destroy = &dx::dx_append_buffer_impl::destroy,
	    },
	};

	return &functions;
}
} // namespace graphics
} // namespace mars
