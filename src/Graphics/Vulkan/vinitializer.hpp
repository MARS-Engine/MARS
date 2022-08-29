#ifndef __VINITIALIZER__
#define __VINITIALIZER__

#include "vtypes.hpp"

class VInitializer {
public:
    static VkPipelineShaderStageCreateInfo pipeline_stage_info(VkShaderStageFlagBits _stage, VkShaderModule _shader_module);
    static VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_info();
    static VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_info(VkPrimitiveTopology _topology);
    static VkPipelineRasterizationStateCreateInfo pipeline_rasterization_info(VkPolygonMode _polygon_mode);
    static VkPipelineMultisampleStateCreateInfo pipeline_multisample_info();
    static VkPipelineColorBlendAttachmentState pipeline_color_blend();
    static VkPipelineLayoutCreateInfo pipeline_layout_info();
    static VkImageCreateInfo image_info(VkFormat _format, VkImageUsageFlags _usage_flags, VkExtent3D _extent);
    static VkImageViewCreateInfo image_view_info(VkFormat _format, VkImage _image, VkImageAspectFlags _aspect_flags);
    static VkPipelineDepthStencilStateCreateInfo depth_stencil_info(bool _b_depth_test, bool _b_depth_write, VkCompareOp _compare_op);
};

#endif