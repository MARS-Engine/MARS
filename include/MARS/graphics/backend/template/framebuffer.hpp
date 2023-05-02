#ifndef MARS_FRAMEBUFFER_
#define MARS_FRAMEBUFFER_

#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include "render_pass.hpp"
#include <MARS/memory/mars_ref.hpp>
#include <vector>

namespace mars_graphics {
    class framebuffer : public graphics_component {
    protected:
        mars_math::vector2<size_t> m_size;
        mars_ref<render_pass> m_render_pass;
        bool m_depth_enabled = false;
        bool m_load_previous = false;
        std::vector<std::shared_ptr<texture>> m_frames;
    public:
        using graphics_component::graphics_component;

        void set_load_previous(bool _load_previous) { m_load_previous = _load_previous; }
        void set_size(const mars_math::vector2<size_t>& _size) { m_size = _size; }
        [[nodiscard]] mars_math::vector2<size_t> get_size() const { return m_size; }
        [[nodiscard]] inline std::shared_ptr<texture> get_texture(size_t _index) const { return _index < m_frames.size() ? m_frames[_index] : std::shared_ptr<texture>(); }

        inline void set_depth(bool _enabled) { m_depth_enabled = _enabled; }
        inline mars_ref<render_pass> get_render_pass() { return m_render_pass; }

        virtual void create(swapchain* _swapchain) {  }
        virtual void create(mars_math::vector2<size_t> _size, const std::vector<std::shared_ptr<texture>>& _textures) { }
        virtual void destroy() { }

        virtual void bind() { }
        virtual void bind_textures() { }
        virtual void unbind() { }
    };
}

#endif