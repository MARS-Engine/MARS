#include <MARS/graphics/backend/opengl/gl_shader_data.hpp>

using namespace mars_graphics;

void gl_shader_data::generate(pipeline* _pipeline, shader* _shader) {
    m_pipeline = _pipeline;
    m_shader = _shader;

    auto uniforms = _shader->get_uniforms();

    for (size_t i = 0; i < uniforms.size(); i++) {
        if (uniforms[i]->type == MARS_UNIFORM_TYPE_SAMPLER)
            continue;

        auto new_buffer = instance()->instance<buffer>();
        new_buffer->create(uniforms[i]->size * sizeof(float), MARS_MEMORY_TYPE_UNIFORM);

        auto uni = new gl_uniform(uniforms[i], i, _shader, new_buffer);
        m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
    }

    for (auto& tex : m_textures)
        tex.second->set_index(_shader->get_uniform(tex.first)->binding);
}