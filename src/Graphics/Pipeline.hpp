#ifndef __PIPELINE__
#define __PIPELINE__

class CommandBuffer;
class VPipeline;
class VEngine;
class Shader;
class VertexInputDescription;

#include <string>

class Pipeline {
public:
    VPipeline* pipeline;
    VEngine* engine;

    Pipeline(VEngine* engine, Shader* shader);

    void CreateLayout(size_t size = 0) const;
    void ApplyInputDescription(VertexInputDescription* description) const;
    void Create() const;
    void Bind(CommandBuffer* commandBuffer) const;
    void UpdateConstant(CommandBuffer* commandBuffer, void* data) const;
};

#endif