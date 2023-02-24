#ifndef MARS_V_INSTANCE_
#define MARS_V_INSTANCE_

#include <vulkan/vulkan.h>
#include "v_base.hpp"
#include <MARS/graphics/backend/vulkan/v_backend/v_debug.hpp>

namespace mars_graphics {

    class v_instance : public v_base {
    private:
        VkDebugUtilsMessengerEXT debugMessenger = nullptr;
        VkInstance m_instance = nullptr;
    public:
        const std::vector<const char*> validation_layers = {
                "VK_LAYER_KHRONOS_validation",
        };

        const std::vector<const char*> device_extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        using v_base::v_base;

        inline VkInstance raw_instance() const { return m_instance;}

        std::vector<const char*> get_required_extensions();
        bool check_validation_layer_support();
        void create();

        inline void destroy() {
            if (graphics()->enable_validation_layer())
                destroy_debug_utils_messenger_ext(raw_instance(), debugMessenger, nullptr);

            vkDestroyInstance(raw_instance(), nullptr);
        }
    };
}

#endif