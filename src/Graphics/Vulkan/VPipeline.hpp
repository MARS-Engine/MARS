#ifndef __VPIPELINE__
#define __VPIPELINE__

class VShader;
class VDevice;
class VRenderPass;
class VSwapchain;
class VertexInputDescription;
class VCommandBuffer;

#include "VTypes.hpp"
#include <vector>

using namespace std;

class VPipeline {
private:
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineLayoutCreateInfo layoutInfo;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineColorBlendStateCreateInfo colorBlending;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
public:
    VShader* shader;
    VDevice* device;
    VSwapchain* swapchain;
    VRenderPass* renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline rawPipeline;
    size_t constantSize;

    VPipeline(VShader* shader, VDevice* device, VSwapchain* swapchain, VRenderPass* renderPass);

    void ApplyInputDescription(VertexInputDescription* description);
    void Create();
    void CreateLayout(size_t size = 0);
    void Bind(VCommandBuffer* commandBuffer) const;
    void UpdateConstant(VCommandBuffer* commandBuffer, void* data) const;
};

#endif