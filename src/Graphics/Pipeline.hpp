#ifndef __PIPELINE__
#define __PIPELINE__

#include <string>
#include "Vulkan/VPipeline.hpp"

class CommandBuffer;
class VEngine;
class Shader;
class VRenderPass;
class VertexInputDescription;

class Pipeline {
public:
    VPipeline* pipeline;
    VEngine* engine;

    Pipeline(VEngine* engine, Shader* shader);
    Pipeline(VEngine* engine, Shader* shader, VRenderPass* renderPass);

    void CreateLayout() const;
    void ApplyInputDescription(VertexInputDescription* description) const;
    void ApplyViewport(PipelineViewport viewport);
    void Create() const;
    void Bind(CommandBuffer* commandBuffer) const;
    void UpdateConstant(CommandBuffer* commandBuffer, void* data) const;
};

#endif