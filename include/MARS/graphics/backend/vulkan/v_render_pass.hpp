#ifndef MARS_V_RENDER_PASS_
#define MARS_V_RENDER_PASS_

#include <MARS/graphics/backend/template/render_pass.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {
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