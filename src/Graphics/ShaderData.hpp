#ifndef __SHADERDATA__
#define __SHADERDATA__

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
    void Generate(size_t bufferSize);
    void Update(void* data);
    void SetTexture(Texture* texture);
};

class ShaderData {
public:
    VShaderData* shaderData;
    Shader* shader;
    VEngine* engine;
    vector<Uniform*> uniforms;

    ShaderData(Shader* shader, VEngine* engine);
    Uniform* GetUniform(string name);
    void Bind(CommandBuffer* commandBuffer, Pipeline* pipeline);
    void Generate();
};

#endif