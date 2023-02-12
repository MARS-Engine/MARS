#ifndef MARS_V_BASE_
#define MARS_V_BASE_

#include <vulkan/vulkan.h>
#include <MARS/graphics/backend/vulkan/v_backend_instance.hpp>

namespace mars_graphics {

    class v_base {
    private:
        v_backend_instance* m_instance;
    public:
        inline v_backend_instance* graphics_instance() const { return m_instance; }

        explicit v_base(v_backend_instance* _instance) {
            m_instance = _instance;
        }
    };
}

#endif