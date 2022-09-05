#ifndef __VPIPELINE__
#define __VPIPELINE__

#include "Math/vector2.hpp"

class VShader;
class vdevice;
class vrender_pass;
class vswapchain;
class VertexInputDescription;
class vcommand_buffer;

#include "vtypes.hpp"
#include <vector>

using namespace std;

struct pipeline_viewport {
    vector2 offset;
    vector2 size = vector2(1, 1);
    vector2 depth = vector2(0, 1);
    bool flip_y = true;
};

class vpipeline {
private:
    //Vulkan values
    vector<VkPipelineColorBlendAttachmentState> color_blend_attachment;
    vector<VkPipelineShaderStageCreateInfo> shader_stages;
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineLayoutCreateInfo layout_info;
    VkPipelineViewportStateCreateInfo viewport_state;
    VkPipelineColorBlendStateCreateInfo color_blending;
    VkPipelineDepthStencilStateCreateInfo depth_stencil;

    //Pipeline
    VkGraphicsPipelineCreateInfo pipeline_info{};
public:
    //MVRE
    vdevice* device = nullptr;

    //Vulkan
    VkViewport viewport;
    VkRect2D scissor;

    //Pipeline
    VkPipelineLayout pipeline_layout;
    VkPipeline raw_pipeline;

    vpipeline(vdevice* _device);

    void apply_input_description(VertexInputDescription* _description);
    void load_render_pass(vrender_pass* _renderPass);
    void load_shader(VShader* _shader);
    void apply_viewport(pipeline_viewport _view_data);
    void create();
    void create_layout();
    void bind(vcommand_buffer* _command_buffer) const;
    void update_constant(vcommand_buffer* _command_buffer, void* _data) const;
};

#endif