#include <MVRE/graphics/backend/vulkan/v_pipeline.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend_instance.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_swapchain.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader.hpp>
#include <MVRE/graphics/backend/vulkan/v_render_pass.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader_input.hpp>

using namespace mvre_graphics;

v_pipeline::v_pipeline(backend_instance* _instance) : pipeline(_instance) {
    m_dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    m_dynamic_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(m_dynamic_states.size()),
            .pDynamicStates = m_dynamic_states.data()
    };

    m_vertex_input_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            //.pVertexBindingDescriptions = &((v_shader_input*)m_shader_input)->raw_binding(),
            //.vertexAttributeDescriptionCount = static_cast<uint32_t>(((v_shader_input*)m_shader_input)->raw_descriptions().size()),
            //.pVertexAttributeDescriptions = ((v_shader_input*)m_shader_input)->raw_descriptions().data()
    };

    m_input_assembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
    };

    m_viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)instance()->get_window()->size().x(),
            .height = (float)instance()->get_window()->size().y(),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };

    m_scissor = {
            .offset = {0, 0},
            .extent = instance<v_backend_instance>()->swapchain()->extent(),
    };

    m_viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &m_viewport,
            .scissorCount = 1,
            .pScissors = &m_scissor
    };

    m_rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f
    };

    m_multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
    };

    m_blend_attachments = {{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD
    }};

    m_color_blending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = static_cast<uint32_t>(m_blend_attachments.size()),
            .pAttachments = m_blend_attachments.data(),
    };
}

void v_pipeline::create() {
    VkPipelineLayoutCreateInfo pipeline_layout_pipeline {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };

    if (vkCreatePipelineLayout(instance<v_backend_instance>()->device()->raw_device(), &pipeline_layout_pipeline, nullptr, &m_pipeline_layout) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Pipeline - Failed to create pipeline layout");

    auto shaderStages = dynamic_cast<v_shader*>(m_shader)->get_stages();

    VkGraphicsPipelineCreateInfo pipeline_info {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &m_vertex_input_info,
        .pInputAssemblyState = &m_input_assembly,
        .pViewportState = &m_viewport_state,
        .pRasterizationState = &m_rasterizer,
        .pMultisampleState = &m_multisampling,
        .pColorBlendState = &m_color_blending,
        .pDynamicState = &m_dynamic_state,
        .layout = m_pipeline_layout,
        .renderPass = ((v_render_pass*)instance<v_backend_instance>()->get_render_pass())->raw_render_pass(),
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE
    };

    if (vkCreateGraphicsPipelines(instance<v_backend_instance>()->device()->raw_device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline) != VK_SUCCESS)
        mvre_debug::debug::error("MVRE - Vulkan - Pipeline - Failed to create pipeline");
}

void v_pipeline::destroy() {
    vkDestroyPipeline(instance<v_backend_instance>()->device()->raw_device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(instance<v_backend_instance>()->device()->raw_device(), m_pipeline_layout, nullptr);
}