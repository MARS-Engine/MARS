#ifndef __MVRE__V__PIPELINE__
#define __MVRE__V__PIPELINE__

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
        VkPipelineMultisampleStateCreateInfo m_multisampling;

        VkViewport m_viewport;
        VkRect2D m_scissor;
    public:
        explicit v_pipeline(backend_instance* _instance);

        void set_viewport(mvre_math::vector4<float> _view, mvre_math::vector2<float> _depth) override;
        void set_extension(mvre_math::vector4<int> _view) override;

        void create() override;
        void destroy() override;
    };
}

#endif