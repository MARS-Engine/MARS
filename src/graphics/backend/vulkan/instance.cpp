#include "mars/graphics/backend/instance.hpp"
#include <cstdint>
#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/instance.hpp>
#include <mars/graphics/backend/window.hpp>
#include <mars/graphics/graphics_engine.hpp>
#include <mars/graphics/window.hpp>
#include <mars/meta.hpp>

#include <vector>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        struct vk_instance_entry : public vk_instance {
            graphics_backend_functions* engine;
        };

        sparse_vector<vk_instance_entry, 2> instances;
        log_channel instance_channel("graphics/vulkan/instance");

        bool check_validation_layer_support(const std::vector<const char*>& _layers) {
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            std::vector<VkLayerProperties> available_layers(layer_count);
            vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

            for (const char* layerName : _layers) {
                bool layerFound = false;

                for (const auto& layerProperties : available_layers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;

            return false;
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity, VkDebugUtilsMessageTypeFlagsEXT _message_type, const VkDebugUtilsMessengerCallbackDataEXT* _callback_data, void* _user_data) {
            vk_instance* user_instance = static_cast<vk_instance*>(_user_data);
            instance new_instance{
                .engine = static_cast<detail::vk_instance_entry*>(user_instance)->engine,
                .data = user_instance,
                .debug_mode = true,
            };

            mars_graphics_message_severity severity = MARS_GRAPHICS_MESSAGE_SEVERITY_ERROR;

            switch (_message_severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                severity = MARS_GRAPHICS_MESSAGE_SEVERITY_VERBOSE;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                severity = MARS_GRAPHICS_MESSAGE_SEVERITY_INFO;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                severity = MARS_GRAPHICS_MESSAGE_SEVERITY_WARN;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                severity = MARS_GRAPHICS_MESSAGE_SEVERITY_ERROR;
                break;
            default:
                logger::error(instance_channel, "invalid vulkan debug callback severity level - {}", meta::enum_to_string(_message_severity));
                break;
            }

            user_instance->broadcast<&vk_instance_events::debug_callback>(new_instance, _callback_data->pMessage, severity);
            return VK_FALSE;
        }

        VkResult vk_create_debug_utils_messenger_ext(VkInstance _instance, const VkDebugUtilsMessengerCreateInfoEXT* _create_info, const VkAllocationCallbacks* _allocator, VkDebugUtilsMessengerEXT* _debug_messenger) {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr)
                return func(_instance, _create_info, _allocator, _debug_messenger);
            else
                return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        void vk_destroy_debug_utils_messenger_ext(VkInstance _instance, VkDebugUtilsMessengerEXT _debug_messenger, const VkAllocationCallbacks* _allocator) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
                func(_instance, _debug_messenger, _allocator);
        }
    } // namespace detail

    instance vk_instance_impl::vk_instance_create(graphics_engine& _engine, const instance_create_params& _params, window& _window) {
        static const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };

        instance new_instance;

        if (_params.debug_mode && !detail::check_validation_layer_support(validation_layers)) {
            logger::error(detail::instance_channel, "failed to get validation layers disabling debug mode");
            new_instance.debug_mode = false;
        } else
            new_instance.debug_mode = _params.debug_mode;

        detail::vk_instance_entry* v_instance = detail::instances.request_entry();

        VkApplicationInfo app_info{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = _params.name.c_str(),
            .applicationVersion = VK_MAKE_VERSION(_params.app_version.x, _params.app_version.y, _params.app_version.z),
            .pEngineName = "MARS",
            .engineVersion = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion = VK_API_VERSION_1_3
        };

        VkInstanceCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info
        };

        std::vector<const char*> extensions;
        _window.engine->get_impl<window_impl>().get_extensions(_window, extensions);

        if (new_instance.debug_mode) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
        }

        create_info.enabledExtensionCount = extensions.size();
        create_info.ppEnabledExtensionNames = extensions.data();

        VkResult result = vkCreateInstance(&create_info, nullptr, &v_instance->instance);

        logger::assert_if(result != VK_SUCCESS, detail::instance_channel, "vkCreateInstance returned {}", meta::enum_to_string(result));

        new_instance.engine = _engine.allocator;
        new_instance.data = v_instance;
        return new_instance;
    }

    void vk_instance_impl::vk_instance_destroy(instance& _instance) {
        vk_instance* ptr = static_cast<vk_instance*>(_instance.data);

        if (ptr->debug_message != VK_NULL_HANDLE)
            detail::vk_destroy_debug_utils_messenger_ext(ptr->instance, ptr->debug_message, nullptr);

        vkDestroyInstance(ptr->instance, nullptr);
    }

    void vk_instance_impl::vk_instance_listen_debug(instance& _instance, void (*_callback)(instance& _instance, const std::string_view& _message, mars_graphics_message_severity _error_severity)) {
        if (!_instance.debug_mode) {
            logger::error(detail::instance_channel, "attempted to debug listen to a vk instance when debug mode is off");
            return;
        }

        vk_instance* ptr = static_cast<vk_instance*>(_instance.data);

        if (ptr->debug_message == VK_NULL_HANDLE) {
            VkDebugUtilsMessengerCreateInfoEXT create_info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = detail::vk_debug_callback,
                .pUserData = _instance.data,
            };

            VkResult result = detail::vk_create_debug_utils_messenger_ext(ptr->instance, &create_info, nullptr, &ptr->debug_message);

            if (result != VK_SUCCESS) {
                logger::error(detail::instance_channel, "failed to listen to vulkans debug callback create_debug_uilts_messenger_ext returned {}", meta::enum_to_string(result));
                return;
            }
        }

        ptr->listen<&vk_instance_events::debug_callback>(_callback);
    }
} // namespace mars::graphics::vulkan