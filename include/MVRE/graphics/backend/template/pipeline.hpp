#ifndef __MVRE__PIPELINE__
#define __MVRE__PIPELINE__

#include "graphics_component.hpp"
#include "shader.hpp"
#include "render_pass.hpp"
#include "shader_input.hpp"

namespace mvre_graphics {

    class pipeline : public graphics_component {
    protected:
        mvre_viewport m_viewport;
        shader* m_shader;
        mvre_shader_inputs m_shader_input;
    public:
        inline shader* get_shader() { return m_shader; }

        using graphics_component::graphics_component;

        inline void set_shader(shader* _shader) { m_shader = _shader; }
        inline void set_shader_input(mvre_shader_inputs _input) { m_shader_input = _input; }

        void set_viewport(mvre_math::vector2<int> _position, mvre_math::vector2<size_t> _size, mvre_math::vector2<float> _depth) {
            m_viewport.position = _position,
            m_viewport.size = _size,
            m_viewport.depth = _depth;
        }

        virtual void set_extension(mvre_math::vector4<int> _view) { }

        virtual void bind() { }
        virtual void create() { }
        virtual void destroy() { }
    };
}

#endif