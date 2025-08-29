#pragma once

#include "vk_device.hpp"
#include "vk_instance.hpp"
#include "vk_pipeline.hpp"
#include "vk_render_pass.hpp"
#include "vk_shader.hpp"
#include "vk_swapchain.hpp"
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
                        .pipeline_destroy = &vulkan::vk_pipeline_impl::vk_pipeline_destroy,
                    },
                    .render_pass{
                        .render_pass_create = &vulkan::vk_render_pass_impl::vk_render_pass_create,
                        .render_pass_destroy = &vulkan::vk_render_pass_impl::vk_render_pass_destroy,
                    },
                };

                return &functions;
            }
        };
    } // namespace graphics
} // namespace mars