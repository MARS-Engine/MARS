#ifndef MARS_PIPELINE_
#define MARS_PIPELINE_

#include "./builders/pipeline_builder.hpp"
#include "graphics_component.hpp"
#include "shader.hpp"
#include "render_pass.hpp"
#include "shader_input.hpp"

namespace mars_graphics {

    class pipeline : public graphics_component {
    protected:
        pipeline_data m_data;

        virtual void create() { }

        inline void set_data(const pipeline_data& _data) {
            m_data = _data;
        }
    public:
        friend pipeline_builder;

        void set_viewport(const mars_math::vector2<int>& _position, const mars_math::vector2<size_t>& _size, const mars_math::vector2<float>& _depth) {
            m_data.viewport.position = _position,
            m_data.viewport.size = _size,
            m_data.viewport.depth = _depth;
        }

        [[nodiscard]] inline mars_ref<shader> get_shader() const { return m_data.pipeline_shader; }

        using graphics_component::graphics_component;

        virtual void bind() { }
    };
}

#endif