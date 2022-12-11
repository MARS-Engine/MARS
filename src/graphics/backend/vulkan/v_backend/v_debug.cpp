#include <MVRE/graphics/backend/vulkan/v_backend/v_debug.hpp>
#include <MVRE/debug/debug.hpp>

using namespace mvre_graphics;

VKAPI_ATTR VkBool32 VKAPI_CALL mvre_graphics::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT _message_severity, VkDebugUtilsMessageTypeFlagsEXT _message_type, const VkDebugUtilsMessengerCallbackDataEXT* _callback_data, void* _user_data) {
    if (_message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        mvre_debug::debug::alert("MVRE - Vulkan - Validation layer: " + (std::string)_callback_data->pMessage);

    return VK_FALSE;
}

VkResult mvre_graphics::create_debug_utils_messenger_ext(VkInstance _instance, const VkDebugUtilsMessengerCreateInfoEXT* _create_info, const VkAllocationCallbacks* _allocator, VkDebugUtilsMessengerEXT* _debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(_instance, _create_info, _allocator, _debug_messenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void mvre_graphics::destroy_debug_utils_messenger_ext(VkInstance _instance, VkDebugUtilsMessengerEXT _debug_messenger, const VkAllocationCallbacks* _allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(_instance, _debug_messenger, _allocator);
}

void mvre_graphics::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& create_info) {
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
}