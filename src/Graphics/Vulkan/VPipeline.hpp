#ifndef __VPIPELINE__
#define __VPIPELINE__

#include "Math/Vector2.hpp"

class VShader;
class VDevice;
class VRenderPass;
class VSwapchain;
class VertexInputDescription;
class VCommandBuffer;

#include "VTypes.hpp"
#include <vector>

using namespace std;

struct PipelineViewport {
    Vector2 offset;
    Vector2 size = Vector2(1, 1);
    Vector2 depth = Vector2(0, 1);
    bool flipY = true;
};

class VPipeline {
private:
    //Vulkan values
    vector<VkPipelineColorBlendAttachmentState> colorBlendAttachment;
    vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineLayoutCreateInfo layoutInfo;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    VkPipelineDepthStencilStateCreateInfo depthStencil;

    //Pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
public:
    //MVRE
    VDevice* device = nullptr;

    //Vulkan
    VkViewport viewport;
    VkRect2D scissor;

    //Pipeline
    VkPipelineLayout pipelineLayout;
    VkPipeline rawPipeline;

    VPipeline(VDevice* device);

    void ApplyInputDescription(VertexInputDescription* description);
    void LoadRenderPass(VRenderPass* renderPass);
    void LoadShader(VShader* shader);
    void ApplyViewport(PipelineViewport viewData);
    void Create();
    void CreateLayout();
    void Bind(VCommandBuffer* commandBuffer) const;
    void UpdateConstant(VCommandBuffer* commandBuffer, void* data) const;
};

#endif