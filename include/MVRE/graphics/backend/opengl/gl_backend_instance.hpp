#ifndef __MVRE__GL__BACKEND__INSTANCE__
#define __MVRE__GL__BACKEND__INSTANCE__

#include <MVRE/graphics/backend/template/backend_instance.hpp>
#include "gl_buffer.hpp"
#include "gl_shader.hpp"
#include "gl_shader_input.hpp"
#include "gl_texture.hpp"
#include "gl_pipeline.hpp"
#include "gl_shader_data.hpp"

namespace mvre_graphics {

    class gl_backend_instance : public backend_instance {
    protected:
        SDL_GLContext m_gl_context = nullptr;

        buffer* generate_buffer() override { return new gl_buffer(this); }
        shader* generate_shader() override { return new gl_shader(this); }
        shader_input* generate_shader_input() override { return new gl_shader_input(this); }
        texture* generate_texture() override { return new gl_texture(this); }
        pipeline* generate_pipeline() override { return new gl_pipeline(this); }
        shader_data* generate_shader_data() override { return new gl_shader_data(this); }

    public:
        using backend_instance::backend_instance;

        void create_with_window(const std::string& _title, mvre_math::vector2<int> _size) override;

        void update() override;
        void prepare_render() override;
        void draw() override;
        void destroy() override;
    };
}

#endif