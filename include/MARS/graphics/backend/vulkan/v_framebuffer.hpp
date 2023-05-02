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
        mars_math::vector2<size_t> m_size;
        v_depth* m_depth;
        bool is_swap = false;

    public:
        using framebuffer::framebuffer;

        [[nodiscard]] inline VkFramebuffer get_frame() const { return m_framebuffers[is_swap ? graphics()->index() : 0]; }

        inline void set_render_pass(const mars_ref<v_render_pass>& _render_pass) { m_render_pass = _render_pass; }

        void create(swapchain* _swapchain) override;
        void create(mars_math::vector2<size_t> _size, const std::vector<std::shared_ptr<texture>>& _textures) override;

        void destroy() override;
    };
}

#endif