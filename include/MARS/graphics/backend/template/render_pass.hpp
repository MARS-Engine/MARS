#ifndef MARS_RENDER_PASS_
#define MARS_RENDER_PASS_

#include "graphics_component.hpp"
#include "framebuffer.hpp"

namespace mars_graphics {

    struct render_pass_attachment {
        MARS_FORMAT format;
        MARS_TEXTURE_LAYOUT layout;
    };

    class render_pass : public graphics_component {
    protected:
        std::vector<render_pass_attachment> attachments;
        framebuffer* m_framebuffer = nullptr;
        bool m_load_previous = true;
    public:
        inline void set_framebuffer(framebuffer* _framebuffer) { m_framebuffer = _framebuffer; }
        inline void set_load_previous(bool _load_previous) { m_load_previous = _load_previous; }

        inline void add_attachment(render_pass_attachment _attachment) { attachments.push_back(_attachment); }
        inline std::vector<render_pass_attachment>& get_attachments() { return attachments; }

        using graphics_component::graphics_component;

        virtual void begin() { }
        virtual void end() { }

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif