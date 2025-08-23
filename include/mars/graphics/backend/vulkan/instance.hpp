#pragma once

#include <mars/event/event.hpp>
#include <mars/graphics/backend/graphics_backend.hpp>
#include <mars/graphics/backend/instance.hpp>
#include <string_view>
#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {
    struct vk_instance_events {
        void debug_callback(instance& _instance, const std::string_view& _message, mars_graphics_message_severity _error_severity);
    };

    struct vk_instance : event<vk_instance_events> {
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_message = VK_NULL_HANDLE;
    };

    struct vk_instance_impl {
        static instance vk_instance_create(graphics_engine& _engine, const instance_create_params& _params, window& _window);
        static void vk_instance_destroy(instance& _instance);
        static void vk_instance_listen_debug(instance& _instance, void (*_callback)(instance& _instance, const std::string_view& _message, mars_graphics_message_severity _error_severity));
    };
} // namespace mars::graphics::vulkan