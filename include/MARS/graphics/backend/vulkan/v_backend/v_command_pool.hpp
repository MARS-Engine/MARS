#ifndef MARS_V_COMMAND_POOL_
#define MARS_V_COMMAND_POOL_

#include "v_base.hpp"
#include "v_device.hpp"

namespace mars_graphics {

    class v_command_pool : public v_base {
    private:
        VkCommandPool m_command_pool;
    public:
        using v_base::v_base;

        [[nodiscard]] inline VkCommandPool raw_command_pool() const { return m_command_pool; }

        void create();

        inline void destroy() {
            vkDestroyCommandPool(graphics()->get_device()->raw_device(), m_command_pool, nullptr);
        }
    };
}

#endif