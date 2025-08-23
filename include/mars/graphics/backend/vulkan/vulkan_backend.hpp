#pragma once

#include "instance.hpp"
#include "window.hpp"

#include <mars/graphics/backend/graphics_backend.hpp>

namespace mars {
    namespace graphics {
        struct vulkan_t {
            inline static graphics_backend_functions* get_functions() {
                static graphics_backend_functions functions{
                    .window = {
                        .window_create = &vulkan::vk_window_impl::window_create,
                        .get_extensions = &vulkan::vk_window_impl::get_extensions,
                    },
                    .instance = {
                        .instance_create = &vulkan::vk_instance_impl::vk_instance_create,
                        .instance_destroy = &vulkan::vk_instance_impl::vk_instance_destroy,
                        .instance_listen_debug = &vulkan::vk_instance_impl::vk_instance_listen_debug,
                    },
                };

                return &functions;
            }
        };
    } // namespace graphics
} // namespace mars