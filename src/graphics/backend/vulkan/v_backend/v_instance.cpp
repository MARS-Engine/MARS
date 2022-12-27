#include <MVRE/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <MVRE/debug/debug.hpp>
#include <SDL2/SDL_vulkan.h>

using namespace mvre_graphics;

std::vector<const char *> v_instance::get_required_extensions() {
    uint32_t extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(graphics_instance()->get_window()->raw_window(), &extensionCount, nullptr);
    std::vector<const char*> extensions(extensionCount);

    SDL_Vulkan_GetInstanceExtensions(graphics_instance()->get_window()->raw_window(), &extensionCount, extensions.data());

    if (graphics_instance()->enable_validation_layer())
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

bool v_instance::check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : validation_layers) {
        bool layer_found = false;

        for (const auto& layerProperties : available_layers) {
            if (strcmp(layer_name, layerProperties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
            return false;
    }

    return true;
}

void v_instance::create() {
    if (graphics_instance()->enable_validation_layer() && !check_validation_layer_support())
        mvre_debug::debug::error("MVRE - Vulkan - Validation Layers Requested but not available!");

    VkApplicationInfo info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "MVRE Sample",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "MVRE",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &info
    };

    auto extensions = get_required_extensions();

    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (graphics_instance()->enable_validation_layer())  {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        createInfo.ppEnabledLayerNames = validation_layers.data();

        populate_debug_messenger_create_info(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
        createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Failed to create a raw_instance");

    if (!graphics_instance()->enable_validation_layer())
        return;

    if (create_debug_utils_messenger_ext(raw_instance(), &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Failed to set up debug messenger");
}