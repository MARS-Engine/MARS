#ifndef __VINITIALIZER__
#define __VINITIALIZER__

#include "VTypes.hpp"

class VInitializer {
public:
    static VkPipelineShaderStageCreateInfo PipelineStageInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
    static VkPipelineVertexInputStateCreateInfo PipelineVertexInputInfo();
    static VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyInfo(VkPrimitiveTopology topology);
    static VkPipelineRasterizationStateCreateInfo PipelineRasterizationInfo(VkPolygonMode polygonMode);
    static VkPipelineMultisampleStateCreateInfo PipelineMultisampleInfo();
    static VkPipelineColorBlendAttachmentState PipelineColorBlend();
    static VkPipelineLayoutCreateInfo PipelineLayoutInfo();
    static VkImageCreateInfo ImageInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    static VkImageViewCreateInfo ImageViewInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
    static VkPipelineDepthStencilStateCreateInfo DepthStencilInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
};

#endif