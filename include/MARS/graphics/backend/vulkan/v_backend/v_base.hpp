#ifndef MARS_V_BASE_
#define MARS_V_BASE_

#include <vulkan/vulkan.h>
#include <MARS/graphics/backend/vulkan/vulkan_backend.hpp>

namespace mars_graphics {

    class v_base {
    private:
        vulkan_backend* m_graphics;
    public:
        [[nodiscard]] inline vulkan_backend* graphics() const { return m_graphics; }

        explicit v_base(vulkan_backend* _graphics) {
            m_graphics = _graphics;
        }
    };
}

#endif