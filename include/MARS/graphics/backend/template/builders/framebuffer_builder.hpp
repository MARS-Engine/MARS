#ifndef MARS_FRAMEBUFFER_BUILDER_
#define MARS_FRAMEBUFFER_BUILDER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "graphics_builder.hpp"
#include <vector>

namespace mars_graphics {
    class framebuffer;
    class swapchain;
    class texture;

    struct framebuffer_data {
        mars_math::vector2<uint32_t> size;
        bool depth_enabled = false;
        bool load_previous = false;
    };

    class framebuffer_builder : graphics_builder<framebuffer> {
    private:
        framebuffer_data m_data;
    public:
        framebuffer_builder() = delete;

        using graphics_builder::graphics_builder;

        inline framebuffer_builder& set_size(const mars_math::vector2<uint32_t>& _size) {
            m_data.size = _size;
            return *this;
        }

        inline framebuffer_builder& set_depth(bool _enabled) {
            m_data.depth_enabled = _enabled;
            return *this;
        }

        inline framebuffer_builder&   set_load_previous(bool _load) {
            m_data.load_previous = _load;
            return *this;
        }

        std::shared_ptr<framebuffer> build(swapchain* _swapchain);
        std::shared_ptr<framebuffer> build(const std::vector<std::shared_ptr<texture>>& _textures);
    };
}

#endif