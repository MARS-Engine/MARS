#ifndef __MVRE__V__RENDER__PASS__
#define __MVRE__V__RENDER__PASS__

#include <MVRE/graphics/backend/template/render_pass.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_render_pass : public render_pass {
    private:
        VkRenderPass m_render_pass;
    public:
        using render_pass::render_pass;

        void begin() override;
        void end() override;

        inline VkRenderPass raw_render_pass() { return m_render_pass; }

        void create() override;
        void destroy() override;
    };
}

#endif