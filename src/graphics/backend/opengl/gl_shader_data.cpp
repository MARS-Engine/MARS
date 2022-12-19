#include <MVRE/graphics/backend/opengl/gl_shader_data.hpp>

using namespace mvre_graphics;

void gl_shader_data::generate(pipeline* _pipeline, shader* _shader) {
    m_pipeline = _pipeline;
    m_shader = _shader;

    auto uniforms = _shader->get_uniforms();

    for (size_t i = 0; i < uniforms.size(); i++) {
        std::vector<buffer*> m_buffers(instance()->max_frames());

        for (auto& new_buffer : m_buffers) {
            new_buffer = instance()->instance<buffer>();
            new_buffer->create(uniforms[i]->size * sizeof(float), MVRE_MEMORY_TYPE_UNIFORM);
        }

        auto uni = new gl_uniform(uniforms[i], i, _shader, m_buffers);
        m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
    }
}