#ifndef MARS_PIPELINE_BUILDER_
#define MARS_PIPELINE_BUILDER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "graphics_builder.hpp"
#include <MARS/memory/mars_ref.hpp>

namespace mars_graphics {
    class pipeline;
    class shader;
    class render_pass;

    struct pipeline_data {
        mars_viewport viewport;
        mars_ref<shader> pipeline_shader;
        std::shared_ptr<render_pass> pipeline_render_pass;
        mars_shader_inputs shader_input;
        bool flip_y = true;
        MARS_TOPOLOGY topology = MARS_TOPOLOGY_TRIANGLE_LIST;
    };

    class pipeline_builder : graphics_builder<pipeline> {
    private:
        pipeline_data m_data;
    public:
        pipeline_builder() = delete;

        using graphics_builder::graphics_builder;

        inline void set_flip_y(bool _flip_y) { m_data.flip_y = _flip_y; }
        inline void set_topology(MARS_TOPOLOGY _topology) { m_data.topology = _topology; }
        inline void set_render_pass(const std::shared_ptr<render_pass>& _render_pass) { m_data.pipeline_render_pass = _render_pass; }
        inline void set_shader(const mars_ref<shader>& _shader) { m_data.pipeline_shader = _shader; }
        inline void set_shader_input(mars_shader_inputs _input) { m_data.shader_input = _input; }

        void set_viewport(const mars_math::vector2<int>& _position, const mars_math::vector2<size_t>& _size, const mars_math::vector2<float>& _depth) {
            m_data.viewport.position = _position,
            m_data.viewport.size = _size,
            m_data.viewport.depth = _depth;
        }

        std::shared_ptr<pipeline> build();
    };
}

#endif