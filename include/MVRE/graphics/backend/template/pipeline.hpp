#ifndef __MVRE__PIPELINE__
#define __MVRE__PIPELINE__

#include "graphics_component.hpp"
#include "shader.hpp"
#include "render_pass.hpp"
#include "shader_input.hpp"

namespace mvre_graphics {

    class pipeline : public graphics_component {
    protected:
        shader* m_shader;
        render_pass* m_render_pass;
        shader_input* m_shader_input;
    public:
        using graphics_component::graphics_component;

        inline void set_shader(shader* _shader) { m_shader = _shader; }
        inline void set_render_pass(render_pass* _pass) { m_render_pass = _pass; }
        inline void set_shader_input(shader_input* _input) { m_shader_input = _input; }

        virtual void set_viewport(mvre_math::vector4<float> _view, mvre_math::vector2<float> _depth) { }
        virtual void set_extension(mvre_math::vector4<int> _view) { }

        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif