#ifndef __MVRE__V__INSTANCE__
#define __MVRE__V__INSTANCE__

#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_instance {
    private:
        VkInstance m_instance = nullptr;
    public:
        inline VkInstance instance() { return m_instance;}

        void create();
    };
}

#endif