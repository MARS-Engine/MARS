#ifndef MVRE_V_DEBUG_
#define MVRE_V_DEBUG_

#include <vulkan/vulkan.h>

namespace mvre_graphics {
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity, VkDebugUtilsMessageTypeFlagsEXT _message_type, const VkDebugUtilsMessengerCallbackDataEXT* _callback_data, void* _user_data);
    VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroy_debug_utils_messenger_ext(VkInstance _instance, VkDebugUtilsMessengerEXT _debug_messenger, const VkAllocationCallbacks* _allocator);
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info);
}

#endif