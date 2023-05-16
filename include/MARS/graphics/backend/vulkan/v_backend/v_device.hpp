#ifndef MARS_V_DEVICE_
#define MARS_V_DEVICE_

#include <MARS/graphics/backend/template/device.hpp>
#include <optional>
#include <vulkan/vulkan.h>

namespace mars_graphics {

    struct swapchain_support_details {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct queue_family_indices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        [[nodiscard]] inline bool is_complete() const {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    class v_device : public device {
    private:
        std::optional<queue_family_indices> m_indices;

        VkPhysicalDevice m_physical_device = nullptr;
        VkDevice m_device = nullptr;
        VkQueue m_graphics_queue = nullptr;
        VkQueue m_present_queue = nullptr;

        queue_family_indices find_queue_families();

        bool check_device_extension_support();

        std::vector<std::shared_ptr<device>> find_devices() override;

        void create() override;

        inline void set_physical(VkPhysicalDevice _physical_device) {
            m_physical_device = _physical_device;
        }
    public:
        using device::device;

        ~v_device();

        bool is_device_suitable();

        [[nodiscard]] inline VkQueue raw_graphics_queue() const { return m_graphics_queue; }
        [[nodiscard]] inline VkQueue raw_present_queue() const { return m_present_queue; }

        [[nodiscard]] inline VkDevice raw_device() const { return m_device; }
        [[nodiscard]] inline VkPhysicalDevice raw_physical_device() const { return m_physical_device; }
        [[nodiscard]] inline queue_family_indices family_indices() const { return m_indices.value(); }

        swapchain_support_details query_swap_chain_support();

        uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}

#endif