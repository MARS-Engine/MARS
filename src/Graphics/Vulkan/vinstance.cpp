#include "vinstance.hpp"

#ifdef NDEBUG
    const bool VInstance::enableValidationLayers = false;
#else
    const bool vinstance::enable_validation_layers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    debug::error(pCallbackData->pMessage);

    return VK_FALSE;
}

void vinstance::create() {
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("").set_engine_name("MVRE").request_validation_layers(enable_validation_layers).require_api_version(1, 1, 0).set_debug_callback(debugCallback).build();
    vkb_instance = inst_ret.value();

    raw_instance = vkb_instance.instance;
    debug_messenger = vkb_instance.debug_messenger;
}

void vinstance::clean() {
    vkb::destroy_debug_utils_messenger(raw_instance, debug_messenger);
    vkDestroyInstance(raw_instance, nullptr);
}