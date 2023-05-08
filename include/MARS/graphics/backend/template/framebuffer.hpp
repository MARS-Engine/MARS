#ifndef MARS_FRAMEBUFFER_
#define MARS_FRAMEBUFFER_

#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include "render_pass.hpp"
#include "./builders/framebuffer_builder.hpp"
#include <MARS/memory/mars_ref.hpp>
#include <vector>


namespace mars_graphics {
    class framebuffer : public graphics_component, public std::enable_shared_from_this<framebuffer> {
    protected:
        framebuffer_data m_data;
        std::vector<std::shared_ptr<texture>> m_frames;
        std::shared_ptr<render_pass> m_render_pass;

        virtual void create(swapchain* _swapchain) {  }
        virtual void create(const std::vector<std::shared_ptr<texture>>& _textures) { }

        inline void set_data(const framebuffer_data& _data) {
            m_data = _data;
        }
    public:
        using graphics_component::graphics_component;

        [[nodiscard]] inline std::shared_ptr<render_pass> get_render_pass() const { return m_render_pass; }
        [[nodiscard]] mars_math::vector2<size_t> get_size() const { return m_data.size; }

        [[nodiscard]] inline std::shared_ptr<texture> get_texture(size_t _index) const { return _index < m_frames.size() ? m_frames[_index] : std::shared_ptr<texture>(); }

        virtual void bind() { }
        virtual void bind_textures() { }
        virtual void unbind() { }

        friend framebuffer_builder;
    };
}

#endif