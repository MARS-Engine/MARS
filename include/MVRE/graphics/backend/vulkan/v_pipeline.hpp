#ifndef MVRE_V_PIPELINE_
#define MVRE_V_PIPELINE_

#include <MVRE/graphics/backend/template/pipeline.hpp>
#include <vulkan/vulkan.h>

namespace mvre_graphics {

    class v_pipeline : public pipeline {
    private:
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;

        VkPipelineColorBlendStateCreateInfo m_color_blending;

        std::vector<VkDynamicState> m_dynamic_states;
        std::vector<VkPipelineColorBlendAttachmentState> m_blend_attachments;

        VkPipelineDynamicStateCreateInfo m_dynamic_state;
        VkPipelineVertexInputStateCreateInfo m_vertex_input_info;
        VkPipelineInputAssemblyStateCreateInfo m_input_assembly;
        VkPipelineViewportStateCreateInfo m_viewport_state;
        VkPipelineRasterizationStateCreateInfo m_rasterizer;
        VkPipelineDepthStencilStateCreateInfo m_depth;
        VkPipelineMultisampleStateCreateInfo m_multisampling;

        VkRect2D m_scissor;

        std::vector<VkDescriptorSet> m_descriptor_set;
    public:
        inline VkPipelineLayout raw_layout() { return m_pipeline_layout; }

        explicit v_pipeline(backend_instance* _instance);

        void bind() override;

        void create() override;
        void destroy() override;
    };
}

#endif