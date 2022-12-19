#ifndef MVRE_V_SYNC_
#define MVRE_V_SYNC_

#include "v_base.hpp"

namespace mvre_graphics {

    class v_sync : v_base {
    private:
        std::vector<VkSemaphore> m_image_available;
        std::vector<VkSemaphore> m_render_finished;
        std::vector<VkFence> m_inflight_fence;
    public:
        using v_base::v_base;

        inline VkSemaphore& image_available() { return  m_image_available[graphics_instance()->current_frame()]; }
        inline VkSemaphore& render_finished() { return  m_render_finished[graphics_instance()->current_frame()]; }
        inline VkFence& inflight_fence() { return  m_inflight_fence[graphics_instance()->current_frame()]; }

        void create();
        void wait();
        void destroy();
    };
}

#endif