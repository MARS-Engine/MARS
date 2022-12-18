#ifndef __MVRE__V__FRAMEBUFFER__
#define __MVRE__V__FRAMEBUFFER__

#include "v_base.hpp"
#include <MVRE/graphics/backend/vulkan/v_render_pass.hpp>

namespace mvre_graphics {

    class v_framebuffer : public v_base {
    private:
        std::vector<VkFramebuffer> m_framebuffers;
        mvre_math::vector2<uint32_t> m_size;
        size_t m_frame_index;
        v_render_pass* m_render_pass;
    public:
        using v_base::v_base;

        ~v_framebuffer() {
            destroy();
        }

        inline void set_frame(size_t _index) { m_frame_index = _index; }
        inline VkFramebuffer get_frame() const { return m_framebuffers[m_frame_index]; }

        inline void set_render_pass(v_render_pass* _render_pass) { m_render_pass = _render_pass; }

        void create(mvre_math::vector2<uint32_t> _size, const std::vector<VkImageView>& _views);

        void destroy();
    };
}

#endif