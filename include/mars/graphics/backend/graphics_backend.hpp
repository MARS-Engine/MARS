#pragma once

#include "buffer.hpp"
#include "command_pool.hpp"
#include "device.hpp"
#include "framebuffer.hpp"
#include "instance.hpp"
#include "pipeline.hpp"
#include "render_pass.hpp"
#include "shader.hpp"
#include "swapchain.hpp"
#include "sync.hpp"
#include "window.hpp"

#include <mars/container/sparse_array.hpp>
#include <mars/graphics/window.hpp>

#include <meta>
#include <utility>

namespace mars {
    struct graphics_backend_functions {
        window_impl window;
        instance_impl instance;
        device_impl device;
        swapchain_impl swapchain;
        shader_impl shader;
        pipeline_impl pipeline;
        render_pass_impl render_pass;
        framebuffer_impl framebuffer;
        command_pool_impl command_pool;
        sync_impl sync;
        buffer_impl buffer;

        template <typename T>
        T& get_impl() {
            constexpr std::meta::access_context ctx = std::meta::access_context::current();
            template for (constexpr auto mem : std::define_static_array(std::meta::nonstatic_data_members_of(^^graphics_backend_functions, ctx))) {
                if constexpr (std::meta::type_of(mem) == ^^T)
                    return this->[:mem:];
            }
            std::unreachable();
        }
    };
}; // namespace mars