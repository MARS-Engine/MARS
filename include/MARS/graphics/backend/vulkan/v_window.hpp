#ifndef V_WINDOW_
#define V_WINDOW_

#include <MARS/graphics/backend/template/window.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {
    class v_instance;

    class v_window : public window {
    private:
        v_instance* m_instance = nullptr;
        VkSurfaceKHR m_surface = nullptr;

        void destroy_surface();
    public:
        using window::window;

        ~v_window() {
            destroy_surface();
            window::~window();
        }

        [[nodiscard]] VkSurfaceKHR raw_surface() const { return m_surface; }

        void create_surface(v_instance* _instance);
    };
}

#endif