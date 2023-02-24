#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_instance.hpp>
#include <set>

using namespace mars_graphics;

queue_family_indices v_device::find_queue_families() {
    queue_family_indices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_families.data());

    for (auto i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphics_family = i;


        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, graphics()->get_vulkan_window()->raw_surface(), &present_support);

        if (present_support)
            indices.present_family = i;

        if (indices.is_complete())
            break;
    }

    return indices;
}

bool v_device::check_device_extension_support() {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(m_physical_device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> requiredExtensions(graphics()->instance()->device_extensions.begin(), graphics()->instance()->device_extensions.end());

    for (const auto& extension : available_extensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

swapchain_support_details v_device::query_swap_chain_support() {
    swapchain_support_details details;

    auto surface = graphics()->get_vulkan_window()->raw_surface();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.present_modes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, surface, &presentModeCount, details.present_modes.data());
    }

    return details;
}

bool v_device::is_device_suitable() {
    if (!m_indices.has_value())
        m_indices = find_queue_families();

    if (check_device_extension_support()) {
        auto swapChainSupport = query_swap_chain_support();
        if (swapChainSupport.formats.empty() || swapChainSupport.present_modes.empty())
            return false;
    }
    else
        return false;

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(m_physical_device, &supportedFeatures);


    return m_indices.value().is_complete() && supportedFeatures.samplerAnisotropy;
}

uint32_t v_device::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;

    mars_debug::debug::error("MARS - Vulkan - Device - Failed to find suitable memory type");
    return 0;
}

void v_device::create() {
    if (!m_indices.has_value())
        m_indices = find_queue_families();

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { m_indices->graphics_family.value(), m_indices->present_family.value() };

    float queuePriority = 1.0f;

    for (uint32_t queueFamily : unique_queue_families)
        queue_create_infos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });

    VkPhysicalDeviceFeatures device_features {
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(graphics()->instance()->device_extensions.size()),
        .ppEnabledExtensionNames = graphics()->instance()->device_extensions.data(),
        .pEnabledFeatures = &device_features
    };

    if (graphics()->enable_validation_layer()) {
        create_info.enabledLayerCount = static_cast<uint32_t>(graphics()->instance()->validation_layers.size());
        create_info.ppEnabledLayerNames = graphics()->instance()->validation_layers.data();
    }
    else
        create_info.enabledLayerCount = 0;

    if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS)
        mars_debug::debug::error("MARS - Vulkan - Failed to create logical device");

    vkGetDeviceQueue(m_device, m_indices.value().graphics_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_indices.value().present_family.value(), 0, &m_present_queue);
}