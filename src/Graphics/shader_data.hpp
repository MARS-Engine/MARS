#ifndef __SHADER__DATA__
#define __SHADER__DATA__

#include <string>
#include <vector>



class vengine;
class texture;
class shader;
struct vuniform;
struct vuniform_data;
class vshader_data;
class command_buffer;
class pipeline;

struct uniform {
public:
    vuniform* raw_uniform;
    vengine* engine;

    uniform(vuniform* _uniform, vengine* _engine);
    void generate(size_t buffer_size) const;
    void update(void* data) const;
    void setTexture(texture* uni_texture) const;
};

class shader_data {
public:
    vshader_data* base_shader_data;
    shader* base_shader;
    vengine* engine;
    std::vector<uniform*> uniforms;

    shader_data(shader* _shader, vengine* _engine);
    uniform* get_uniform(const std::string& _name);
    void change_texture(const std::string& _name, texture* _texture) const;
    void bind(command_buffer* _command_buffer, pipeline* _pipeline) const;
    void generate() const;
};

#endif