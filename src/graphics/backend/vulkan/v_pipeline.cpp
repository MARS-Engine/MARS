#include <MVRE/graphics/backend/vulkan/v_pipeline.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend_instance.hpp>
#include <MVRE/graphics/backend/vulkan/v_backend/v_swapchain.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader.hpp>
#include <MVRE/graphics/backend/vulkan/v_render_pass.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader_input.hpp>

using namespace mvre_graphics;

void v_pipeline::create_descriptor_sets() {
    std::vector<VkDescriptorSetLayout> layouts(instance()->max_frames(), ((v_shader*)m_shader)->raw_uniform_layout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = ((v_shader*)m_shader)->raw_descriptor_pool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(instance()->max_frames());
    allocInfo.pSetLayouts = layouts.data();

    m_descriptor_set.resize(instance()->max_frames());
    if (vkAllocateDescriptorSets(instance<v_backend_instance>()->device()->raw_device(), &allocInfo, m_descriptor_set.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < instance()->max_frames(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        //bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        //bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptor_set[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(instance<v_backend_instance>()->device()->raw_device(), 1, &descriptorWrite, 0, nullptr);
    }
}

void v_pipeline::bind() {
    auto command_buffer = instance<v_backend_instance>()->raw_command_buffer();
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkViewport viewport{};
    viewport.x = (float)m_viewport.position.x();
    viewport.y = (float)m_viewport.position.y();
    viewport.width = (float) m_viewport.size.x();
    viewport.height = (float) m_viewport.size.y();
    viewport.minDepth = m_viewport.depth.x();
    viewport.maxDepth = m_viewport.depth.y();
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    vkCmdSetScissor(command_buffer, 0, 1, &m_scissor);

    //vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
}

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

    m_input_assembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
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
            .pViewports = &viewport,
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

    std::vector<VkVertexInputAttributeDescription> descriptions;

    uint32_t binding_stride = 0;

    for (auto i = 0; i < m_shader_input.length; i++) {
        binding_stride += m_shader_input.input_data[i].stride * sizeof(float);

        VkVertexInputAttributeDescription new_dec{
            .location = static_cast<uint32_t>(i),
            .binding = 0,
            .offset = m_shader_input.input_data[i].offset,
        };

        switch (m_shader_input.input_data[i].type) {
            case MVRE_SHADER_INPUT_TYPE_SF_RG:
                new_dec.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case MVRE_SHADER_INPUT_TYPE_SF_RGB:
                new_dec.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case MVRE_SHADER_INPUT_TYPE_SF_RGBA:
                new_dec.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
        }

        descriptions.push_back(new_dec);
    }

    VkVertexInputBindingDescription binding = {
            .binding = 0,
            .stride = binding_stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    m_vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(descriptions.size()),
        .pVertexAttributeDescriptions = descriptions.data()
    };

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