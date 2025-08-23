#pragma once

#include <mars/graphics/backend/window.hpp>

namespace mars::graphics::vulkan {
    struct vk_window_impl {
        static window window_create(graphics_engine& _engine, const window_params& _params);
        static void get_extensions(const window& _window, std::vector<const char*>& _extensions);
    };
} // namespace mars::graphics::vulkan