#pragma once

#include "buffer.hpp"
#include "device.hpp"
#include "instance.hpp"
#include "pipeline.hpp"
#include "render_pass.hpp"
#include "shader.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <mars/container/sparse_array.hpp>
#include <mars/graphics/window.hpp>

#include <meta>
#include <utility>

namespace mars {
    struct graphics_backend_functions {
        buffer_impl buffer;
        window_impl window;
        instance_impl instance;
        device_impl device;
        swapchain_impl swapchain;
        shader_impl shader;
        pipeline_impl pipeline;
        render_pass_impl render_pass;

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