#ifndef MARS_RENDERER_
#define MARS_RENDERER_

#include "MARS/graphics/backend/template/framebuffer.hpp"
#include "MARS/graphics/backend/template/render_pass.hpp"
#include "MARS/graphics/backend/template/texture.hpp"
#include <vector>

namespace mars_graphics {
    class graphics_backend;

    enum RENDERER_TEXTURE_TYPE {
        RENDERER_TEXTURE_TYPE_FRAME,
        RENDERER_TEXTURE_TYPE_COLOR,
        RENDERER_TEXTURE_TYPE_POSITION,
        RENDERER_TEXTURE_TYPE_FORWARD,
        RENDERER_TEXTURE_TYPE_DEPTH
    };

    struct renderer_frame_data {
        mars_ref<framebuffer> frame;
        std::vector<mars_ref<texture>> buffers;
    };

    class renderer : graphics_component {
    private:
        std::map<std::string, renderer_frame_data> m_framebuffers;
        std::string m_render_type;
    public:
        [[nodiscard]] inline mars_ref<framebuffer> get_framebuffer(const std::string& name) { return m_framebuffers[name].frame; }
        [[nodiscard]] inline std::string get_render_type() const { return m_render_type; }

        using graphics_component::graphics_component;

        void create(const std::string& _path);

        void destroy();
    };
}

#endif