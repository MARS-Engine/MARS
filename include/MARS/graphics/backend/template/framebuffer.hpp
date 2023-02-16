#ifndef MARS_FRAMEBUFFER_
#define MARS_FRAMEBUFFER_

#include "graphics_component.hpp"
#include "graphics_types.hpp"
#include "render_pass.hpp"

namespace mars_graphics {
    class framebuffer : public graphics_component {
    protected:
        mars_math::vector2<size_t> m_size;
        render_pass* m_render_pass = nullptr;
        bool m_depth_enabled = false;
        bool m_load_previous = false;
        std::vector<texture*> m_frames;
    public:
        using graphics_component::graphics_component;

        void set_load_previous(bool _load_previous) { m_load_previous = _load_previous; }
        void set_size(const mars_math::vector2<size_t>& _size) { m_size = _size; }
        [[nodiscard]] mars_math::vector2<size_t> get_size() const { return m_size; }
        [[nodiscard]] inline texture* get_texture(size_t _index) const { return _index < m_frames.size() ? m_frames[_index] : nullptr; }

        inline void set_depth(bool _enabled) { m_depth_enabled = _enabled; }
        inline render_pass* get_render_pass() { return m_render_pass; }

        virtual void create(swapchain* _swapchain) {  }
        virtual void create(mars_math::vector2<size_t> _size, const std::vector<texture*>& _textures) { }
        virtual void destroy() { }

        virtual void bind() { }
        virtual void bind_textures() { }
        virtual void unbind() { }
    };
}

#endif