#pragma once

#include "vk_buffer.hpp"
#include "vk_command_pool.hpp"
#include "vk_descriptor.hpp"
#include "vk_device.hpp"
#include "vk_framebuffer.hpp"
#include "vk_instance.hpp"
#include "vk_pipeline.hpp"
#include "vk_render_pass.hpp"
#include "vk_shader.hpp"
#include "vk_swapchain.hpp"
#include "vk_sync.hpp"
#include "vk_window.hpp"

#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    namespace graphics {
        struct vulkan_t {
            inline static graphics_backend_functions* get_functions() {
                static graphics_backend_functions functions{
                    .window = {
                        .window_create = &vulkan::vk_window_impl::vk_window_create,
                        .window_create_surface = &vulkan::vk_window_impl::vk_window_create_surface,
                        .window_get_instance_extensions = &vulkan::vk_window_impl::vk_window_get_instance_extensions,
                        .window_get_device_extensions = &vulkan::vk_window_impl::vk_window_get_device_extensions,
                        .window_destroy_surface = &vulkan::vk_window_impl::vk_window_destroy_surface,
                        .window_destroy = &vulkan::vk_window_impl::vk_window_destroy,
                    },
                    .instance = {
                        .instance_create = &vulkan::vk_instance_impl::vk_instance_create,
                        .instance_destroy = &vulkan::vk_instance_impl::vk_instance_destroy,
                        .instance_listen_debug = &vulkan::vk_instance_impl::vk_instance_listen_debug,
                    },
                    .device = {
                        .device_create = &vulkan::vk_device_impl::vk_device_create,
                        .device_submit_graphics_queue = &vulkan::vk_device_impl::vk_device_submit_graphics_queue,
                        .device_present = &vulkan::vk_device_impl::vk_device_present,
                        .device_wait = &vulkan::vk_device_impl::vk_device_wait,
                        .device_destroy = &vulkan::vk_device_impl::vk_device_destroy,
                    },
                    .swapchain = {
                        .swapchain_create = &vulkan::vk_swapchain_impl::vk_swapchain_create,
                        .swapchain_destroy = &vulkan::vk_swapchain_impl::vk_swapchain_destroy,
                    },
                    .shader = {
                        .shader_create = &vulkan::vk_shader_impl::vk_shader_create,
                        .shader_destroy = &vulkan::vk_shader_impl::vk_shader_destroy,
                    },
                    .pipeline = {
                        .pipeline_create = &vulkan::vk_pipeline_impl::vk_pipeline_create,
                        .pipeline_bind = &vulkan::vk_pipeline_impl::vk_pipeline_bind,
                        .pipeline_destroy = &vulkan::vk_pipeline_impl::vk_pipeline_destroy,
                    },
                    .render_pass{
                        .render_pass_create = &vulkan::vk_render_pass_impl::vk_render_pass_create,
                        .render_pass_bind = &vulkan::vk_render_pass_impl::vk_render_pass_bind,
                        .render_pass_unbind = &vulkan::vk_render_pass_impl::vk_render_pass_unbind,
                        .render_pass_destroy = &vulkan::vk_render_pass_impl::vk_render_pass_destroy,
                    },
                    .framebuffer{
                        .framebuffer_create = &vulkan::vk_framebuffer_impl::vk_framebuffer_create,
                        .framebuffer_destroy = &vulkan::vk_framebuffer_impl::vk_framebuffer_destroy,
                    },
                    .command_pool{
                        .command_pool_create = &vulkan::vk_command_pool_impl::vk_command_pool_create,
                        .command_buffer_create = &vulkan::vk_command_pool_impl::vk_command_buffer_create,
                        .command_buffer_reset = &vulkan::vk_command_pool_impl::vk_command_buffer_reset,
                        .command_buffer_record = &vulkan::vk_command_pool_impl::vk_command_buffer_record,
                        .command_buffer_record_end = &vulkan::vk_command_pool_impl::vk_command_buffer_record_end,
                        .command_buffer_draw = &vulkan::vk_command_pool_impl::vk_command_buffer_draw,
                        .command_buffer_draw_indexed = &vulkan::vk_command_pool_impl::vk_command_buffer_draw_indexed,
                        .command_pool_destroy = &vulkan::vk_command_pool_impl::vk_command_pool_destroy,
                    },
                    .sync{
                        .sync_create = &vulkan::vk_sync_impl::vk_sync_create,
                        .sync_wait = &vulkan::vk_sync_impl::vk_sync_wait,
                        .sync_reset = &vulkan::vk_sync_impl::vk_sync_reset,
                        .sync_get_next_image = &vulkan::vk_sync_impl::vk_sync_get_next_image,
                        .sync_destroy = &vulkan::vk_sync_impl::vk_sync_destroy,
                    },
                    .buffer{
                        .buffer_create = &vulkan::vk_buffer_impl::vk_buffer_create,
                        .buffer_bind = &vulkan::vk_buffer_impl::vk_buffer_bind,
                        .buffer_bind_index = &vulkan::vk_buffer_impl::vk_buffer_bind_index,
                        .buffer_copy = &vulkan::vk_buffer_impl::vk_buffer_copy,
                        .buffer_map = &vulkan::vk_buffer_impl::vk_buffer_map,
                        .buffer_unmap = &vulkan::vk_buffer_impl::vk_buffer_unmap,
                        .buffer_destroy = &vulkan::vk_buffer_impl::vk_buffer_destroy,
                    },
                    .descritor{
                        .descriptor_create = &vulkan::vk_descriptor_impl::vk_descriptor_create,
                        .descriptor_set_create = &vulkan::vk_descriptor_impl::vk_descriptor_set_create,
                        .descriptor_set_bind = &vulkan::vk_descriptor_impl::vk_descriptor_set_bind,
                        .descriptor_destroy = &vulkan::vk_descriptor_impl::vk_descriptor_destroy,
                    },
                };

                return &functions;
            }
        };
    } // namespace graphics
} // namespace mars