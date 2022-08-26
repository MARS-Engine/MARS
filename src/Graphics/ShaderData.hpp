#ifndef __SHADER__DATA__
#define __SHADER__DATA__

#include <string>
#include <vector>

using namespace std;

class VEngine;
class Texture;
class Shader;
struct VUniform;
struct VUniformData;
class VShaderData;
class CommandBuffer;
class Pipeline;

struct Uniform {
    VUniform* uniform;
    VEngine* engine;

    Uniform(VUniform* uni, VEngine* engine);
    void Generate(size_t bufferSize) const;
    void Update(void* data) const;
    void SetTexture(Texture* texture) const;
};

class ShaderData {
public:
    VShaderData* shaderData;
    Shader* shader;
    VEngine* engine;
    vector<Uniform*> uniforms;

    ShaderData(Shader* shader, VEngine* engine);
    Uniform* GetUniform(const string& name);
    void ChangeTexture(const string& name, Texture* texture) const;
    void Bind(CommandBuffer* commandBuffer, Pipeline* pipeline) const;
    void Generate() const;
};

#endif