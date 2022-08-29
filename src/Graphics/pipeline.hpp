#ifndef __PIPELINE__
#define __PIPELINE__

#include <string>
#include "Vulkan/vpipeline.hpp"

class command_buffer;
class vengine;
class shader;
class vrender_pass;
class VertexInputDescription;

class pipeline {
public:
    vpipeline* base_pipeline;
    vengine* engine;

    pipeline(vengine* _engine, shader* _shader);
    pipeline(vengine* _engine, shader* _shader, vrender_pass* render_pass);

    void create_layout() const;
    void apply_input_description(VertexInputDescription* _description) const;
    void apply_viewport(pipeline_viewport _viewport);
    void create() const;
    void bind(command_buffer* _command_buffer) const;
    void update_constant(command_buffer* _command_buffer, void* _data) const;
};

#endif