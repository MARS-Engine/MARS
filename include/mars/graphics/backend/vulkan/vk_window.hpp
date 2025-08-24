#pragma once

#include <mars/graphics/backend/window.hpp>

namespace mars {
    struct instance;

    namespace graphics::vulkan {
        struct vk_window_impl {
            static window vk_window_create(graphics_engine& _engine, const window_params& _params);
            static void vk_window_create_surface(window& _window, instance& _instance);
            static void vk_window_get_extensions(const window& _window, std::vector<const char*>& _extensions);
            static void vk_window_destroy_surface(window& _window, instance& _instance);
            static void vk_window_destroy(window& _window);
        };
    } // namespace graphics::vulkan
} // namespace mars