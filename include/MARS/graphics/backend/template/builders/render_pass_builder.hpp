#ifndef MARS_RENDER_PASS_BUILDER_
#define MARS_RENDER_PASS_BUILDER_

#include <MARS/memory/mars_ref.hpp>
#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "../graphics_builder.hpp"
#include <vector>

namespace mars_graphics {
    class render_pass;
    class framebuffer;

    struct render_pass_attachment {
        MARS_FORMAT format;
        MARS_TEXTURE_LAYOUT layout;
    };

    struct render_pass_data {
        mars_ref<framebuffer> framebuffer_ptr;
        bool load_previous;
        std::vector<render_pass_attachment> attachments;
    };

    class render_pass_builder : graphics_builder<render_pass> {
    private:
        render_pass_data m_data;
    public:
        render_pass_builder() = delete;

        using graphics_builder::graphics_builder;

        inline render_pass_builder& set_load_previous(bool _load_previous) {
            m_data.load_previous = _load_previous;
            return *this;
        }

        inline render_pass_builder& set_framebuffer(const mars_ref<framebuffer>& _framebuffer) {
            m_data.framebuffer_ptr = _framebuffer;
            return *this;
        }

        inline render_pass_builder& add_attachment(const render_pass_attachment& _attachment) {
            m_data.attachments.push_back(_attachment);
            return *this;
        }

        std::shared_ptr<render_pass> build();
    };
}

#endif