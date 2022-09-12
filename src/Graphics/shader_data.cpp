#include "shader_data.hpp"
#include "vengine.hpp"
#include "texture.hpp"
#include "pipeline.hpp"
#include "shader.hpp"
#include "Vulkan/vshader_data.hpp"
#include "Vulkan/vshader.hpp"

uniform::uniform(vuniform* _uniform, vengine* _engine) {
    engine = _engine;
    raw_uniform = _uniform;
}

void uniform::generate(size_t buffer_size) const {
    raw_uniform->generate(buffer_size, vengine::frame_overlap);
}

void uniform::update(void* data) const {
    raw_uniform->update(data, engine->render_frame);
}
void uniform::setTexture(texture* uni_texture) const {
    raw_uniform->set_texture(uni_texture->base_texture);
}

shader_data::shader_data(shader* _shader, vengine* _engine) {
    base_shader = _shader;
    engine = _engine;

    base_shader_data = new vshader_data(base_shader->base_shader, engine->allocator);
    for (auto uni : base_shader_data->uniforms)
        uniforms.push_back(new uniform(uni, engine));
}

uniform* shader_data::get_uniform(const std::string& _name) {
    for (auto uni : uniforms)
        if (uni->raw_uniform->data->name == _name)
            return uni;
    debug::error("Uniform - Failed to find uniform with name - " + _name);
    return nullptr;
}

void shader_data::change_texture(const std::string& _name, texture* _texture) const {
    base_shader_data->change_texture(_name, _texture->base_texture);
}

void shader_data::bind(command_buffer* _command_buffer, pipeline* _pipeline) const {
    base_shader_data->bind(_command_buffer->base_command_buffer, _pipeline->base_pipeline);
}

void shader_data::generate() const {
    base_shader_data->generate(vengine::frame_overlap);
}