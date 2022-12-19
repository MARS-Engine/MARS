#ifndef MVRE_V_DEVICE_
#define MVRE_V_DEVICE_

#include "v_base.hpp"
#include <optional>

namespace mvre_graphics {

    struct swapchain_support_details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct queue_family_indices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        inline bool is_complete() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    class v_device : public v_base {
    private:
        std::optional<queue_family_indices> m_indices;

        VkPhysicalDevice m_physical_device = nullptr;
        VkDevice m_device = nullptr;
        VkQueue m_graphics_queue = nullptr;
        VkQueue m_present_queue = nullptr;

        queue_family_indices find_queue_families();

        bool check_device_extension_support();

    public:
        bool is_device_suitable();

        inline VkQueue raw_graphics_queue() const { return m_graphics_queue; }
        inline VkQueue raw_present_queue() const { return m_present_queue; }

        inline VkDevice raw_device() const { return m_device; }
        inline VkPhysicalDevice raw_physical_device() const { return m_physical_device; }
        inline queue_family_indices family_indices() const { return m_indices.value(); }

        swapchain_support_details query_swap_chain_support();

        uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        v_device(v_backend_instance* _instance, VkPhysicalDevice _physical_device) : v_base(_instance) {
            m_physical_device = _physical_device;
        }

        void create();

        inline void destroy()  {
            if (m_device != nullptr)
                vkDestroyDevice(m_device, nullptr);
        }
    };
}

#endif