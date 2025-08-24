#pragma once

#include "buffer.hpp"
#include "device.hpp"
#include "instance.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <mars/container/sparse_array.hpp>
#include <mars/graphics/window.hpp>
#include <meta>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace mars {
    struct graphics_backend_functions {
        buffer_impl buffer;
        window_impl window;
        instance_impl instance;
        device_impl device;
        swapchain_impl swapchain;

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