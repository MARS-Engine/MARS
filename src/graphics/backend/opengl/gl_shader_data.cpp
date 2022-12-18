#include <MVRE/graphics/backend/opengl/gl_shader_data.hpp>

using namespace mvre_graphics;

void gl_shader_data::generate(shader* _shader) {
    auto uniforms = _shader->get_uniforms();
    for (size_t i = 0; i < uniforms.size(); i++) {
        auto new_buffer  = instance()->instance<buffer>();
        new_buffer->create(uniforms[i]->size * sizeof(float), MVRE_MEMORY_TYPE_UNIFORM);

        auto uni = new gl_uniform(i, _shader, new_buffer);
        m_uniforms.insert(std::make_pair(uniforms[i]->name, (uniform*)uni));
    }
}