#include "VInstance.hpp"

#ifdef NDEBUG
    const bool VInstance::enableValidationLayers = false;
#else
    const bool VInstance::enableValidationLayers = true;
#endif

void VInstance::Create() {
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("").request_validation_layers(enableValidationLayers).require_api_version(1, 1, 0).use_default_debug_messenger().build();
    vkbInstance = inst_ret.value();

    rawInstance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;
}

void VInstance::Clean() {
    vkb::destroy_debug_utils_messenger(rawInstance, debugMessenger);
    vkDestroyInstance(rawInstance, nullptr);
}