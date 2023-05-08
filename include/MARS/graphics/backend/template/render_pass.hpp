#ifndef MARS_RENDER_PASS_
#define MARS_RENDER_PASS_

#include "graphics_component.hpp"
#include "framebuffer.hpp"
#include "builders/render_pass_builder.hpp"
#include <vector>

namespace mars_graphics {

    class render_pass : public graphics_component {
    protected:
        render_pass_data m_data;

        void set_data(const render_pass_data& _data) { m_data = _data; }

        virtual void create() { }
    public:
        using graphics_component::graphics_component;

        friend render_pass_builder;

        inline std::vector<render_pass_attachment>& get_attachments() { return m_data.attachments; }

        virtual void begin() { }
        virtual void end() { }
    };
}

#endif