#include "pipeline.hpp"
#include "vengine.hpp"
#include "shader.hpp"
#include "command_buffer.hpp"
#include "Vulkan/vpipeline.hpp"

pipeline::pipeline(vengine* _engine, shader* _shader) {
    _engine = _engine;
    base_pipeline = new vpipeline(_engine->device);
    base_pipeline->load_shader(_shader->base_shader);
    base_pipeline->load_render_pass(_engine->renderPass);
}

pipeline::pipeline(vengine* _engine, shader* _shader, vrender_pass* render_pass) {
    _engine = _engine;
    base_pipeline = new vpipeline(_engine->device);
    base_pipeline->load_shader(_shader->base_shader);
    base_pipeline->load_render_pass(render_pass);
}

void pipeline::create_layout() const {
    base_pipeline->create_layout();
}

void pipeline::apply_input_description(VertexInputDescription* _description) const {
    base_pipeline->apply_input_description(_description);
}

void pipeline::apply_viewport(pipeline_viewport _viewport) {
    base_pipeline->apply_viewport(_viewport);
}

void pipeline::create() const {
    base_pipeline->create();
}

void pipeline::bind(command_buffer* _command_buffer) const {
    base_pipeline->bind(_command_buffer->base_command_buffer);
}

void pipeline::update_constant(command_buffer* _command_buffer, void* _data) const {
    base_pipeline->update_constant(_command_buffer->base_command_buffer, _data);
}