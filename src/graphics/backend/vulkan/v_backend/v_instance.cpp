#include <MVRE/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <MVRE/debug/debug.hpp>
#include <SDL2/SDL_vulkan.h>

using namespace mvre_graphics;

void v_instance::create() {
    VkApplicationInfo info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "MVRE_SAMPLE",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "MVRE",
        .engineVersion = VK_MAKE_VERSION(0, 1, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &info
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    //SDL_Vulkan_GetInstanceExtensions();

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create g_instance!");
    }
}