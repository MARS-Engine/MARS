#ifndef MARS_V_PIPELINE_
#define MARS_V_PIPELINE_

#include <MARS/graphics/backend/template/pipeline.hpp>
#include <vulkan/vulkan.h>

namespace mars_graphics {

    class v_pipeline : public pipeline {
    private:
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline;

        std::vector<VkDynamicState> m_dynamic_states;

        VkRect2D m_scissor;

        std::vector<VkDescriptorSet> m_descriptor_set;
    public:
        inline VkPipelineLayout raw_layout() { return m_pipeline_layout; }

        using pipeline::pipeline;

        void bind() override;

        void create() override;
        void destroy() override;
    };
}

#endif