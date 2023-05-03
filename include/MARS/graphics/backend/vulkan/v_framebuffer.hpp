#ifndef MARS_V_FRAMEBUFFER_
#define MARS_V_FRAMEBUFFER_

#include <MARS/graphics/backend/template/framebuffer.hpp>
#include <MARS/graphics/backend/template/swapchain.hpp>
#include "v_render_pass.hpp"

namespace mars_graphics {

    class v_depth;

    class v_framebuffer : public framebuffer {
    private:
        std::vector<VkFramebuffer> m_framebuffers;
        v_depth* m_depth;
        bool is_swap = false;

        void create(swapchain* _swapchain) override;

        void create(const std::vector<std::shared_ptr<texture>>& _textures) override;
    public:
        using framebuffer::framebuffer;

        ~v_framebuffer();

        [[nodiscard]] inline VkFramebuffer get_frame() const { return m_framebuffers[is_swap ? graphics()->index() : 0]; }
    };
}

#endif