#include "VInstance.hpp"

#ifdef NDEBUG
    const bool VInstance::enableValidationLayers = false;
#else
    const bool VInstance::enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    Debug::Error(pCallbackData->pMessage);

    return VK_FALSE;
}

void VInstance::Create() {
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("").set_engine_name("MVRE").request_validation_layers(enableValidationLayers).require_api_version(1, 1, 0).set_debug_callback(debugCallback).build();
    vkbInstance = inst_ret.value();

    rawInstance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;
}

void VInstance::Clean() {
    vkb::destroy_debug_utils_messenger(rawInstance, debugMessenger);
    vkDestroyInstance(rawInstance, nullptr);
}