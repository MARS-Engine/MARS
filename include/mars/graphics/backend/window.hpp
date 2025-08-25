#pragma once

#include <mars/math/vector2.hpp>

#include <vector>
#include <vulkan/vulkan.h>

namespace mars {
    struct window;
    struct instance;

    class graphics_engine;

    struct window_params {
        std::string title;
        mars::vector2<size_t> size;
    };

    struct vk_window {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
    };

    struct window_impl {
        window (*window_create)(graphics_engine& _engine, const window_params& _params) = nullptr;
        void (*window_create_surface)(window& _window, instance& _instance) = nullptr;
        void (*window_get_instance_extensions)(const window& _window, std::vector<const char*>& _extensions) = nullptr;
        void (*window_get_device_extensions)(const window& _window, std::vector<const char*>& _extensions) = nullptr;
        void (*window_destroy_surface)(window& _window, instance& _instance) = nullptr;
        void (*window_destroy)(window& _window) = nullptr;
    };
} // namespace mars