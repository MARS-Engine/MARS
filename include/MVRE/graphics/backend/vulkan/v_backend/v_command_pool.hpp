#ifndef __MVRE__V__COMMAND__POOL__
#define __MVRE__V__COMMAND__POOL__

#include "v_base.hpp"
#include "v_device.hpp"

namespace mvre_graphics {

    class v_command_pool : public v_base {
    private:
        VkCommandPool m_command_pool;
    public:
        using v_base::v_base;

        inline VkCommandPool raw_command_pool() const { return m_command_pool; }

        ~v_command_pool() {
            destroy();
        }

        void create();

        inline void destroy() {
            vkDestroyCommandPool(graphics_instance()->device()->raw_device(), m_command_pool, nullptr);
        }
    };
}

#endif