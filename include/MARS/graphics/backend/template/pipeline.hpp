#ifndef MARS_PIPELINE_
#define MARS_PIPELINE_

#include "graphics_component.hpp"
#include "shader.hpp"
#include "render_pass.hpp"
#include "shader_input.hpp"

namespace mars_graphics {

    class pipeline : public graphics_component {
    protected:
        mars_viewport m_viewport;
        shader* m_shader = nullptr;
        render_pass* m_render_pass = nullptr;
        mars_shader_inputs m_shader_input;
        bool m_flip_y = true;
        MARS_TOPOLOGY m_topology = MARS_TOPOLOGY_TRIANGLE_LIST;
    public:
        inline shader* get_shader() const { return m_shader; }

        using graphics_component::graphics_component;

        inline void set_flip_y(bool _flip_y) { m_flip_y = _flip_y; }
        inline void set_topology(MARS_TOPOLOGY _topology) { m_topology = _topology; }
        inline void set_render_pass(render_pass* _render_pass) { m_render_pass = _render_pass; }
        inline void set_shader(shader* _shader) { m_shader = _shader; }
        inline void set_shader_input(mars_shader_inputs _input) { m_shader_input = _input; }

        void set_viewport(mars_math::vector2<int> _position, mars_math::vector2<size_t> _size, mars_math::vector2<float> _depth) {
            m_viewport.position = _position,
            m_viewport.size = _size,
            m_viewport.depth = _depth;
        }

        virtual void set_extension(mars_math::vector4<int> _view) { }

        virtual void bind() { }

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif