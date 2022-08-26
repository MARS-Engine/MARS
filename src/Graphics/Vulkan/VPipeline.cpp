#include "VPipeline.hpp"
#include "VShader.hpp"
#include "VDevice.hpp"
#include "VRenderPass.hpp"
#include "VInitializer.hpp"
#include "VSwapchain.hpp"
#include "VCommandBuffer.hpp"

VPipeline::VPipeline(VShader* _shader, VDevice* _device, VSwapchain* _swapchain, VRenderPass* _renderPass) {
    device = _device;
    renderPass = _renderPass;
    swapchain = _swapchain;
    shader = _shader;

    layoutInfo = VInitializer::PipelineLayoutInfo();
    vertexInputInfo = VInitializer::PipelineVertexInputInfo();
    inputAssembly = VInitializer::PipelineInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    rasterizationInfo = VInitializer::PipelineRasterizationInfo(VK_POLYGON_MODE_FILL);
    multisampleInfo = VInitializer::PipelineMultisampleInfo();

    for (auto a : renderPass->attachments) {
        if (a.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            continue;
        VkPipelineColorBlendAttachmentState color = VInitializer::PipelineColorBlend();
        colorBlendAttachment.push_back(color);
    }

    depthStencil = VInitializer::DepthStencilInfo(true, true, VK_COMPARE_OP_LESS);

    viewport.x = 0.0f;
    viewport.y = swapchain->size.y;
    viewport.width = swapchain->size.x;
    viewport.height = -swapchain->size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = { 0, 0 };
    scissor.extent = { (uint32_t)swapchain->size.x, (uint32_t)swapchain->size.y };
}

void VPipeline::CreateLayout(size_t size) {
    constantSize = size;

    if (constantSize != 0) {
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = size;
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        layoutInfo.pPushConstantRanges = &push_constant;
        layoutInfo.pushConstantRangeCount = 1;
    }

    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &shader->layout;

    VK_CHECK(vkCreatePipelineLayout(device->rawDevice, &layoutInfo, nullptr, &pipelineLayout));
}

void VPipeline::ApplyInputDescription(VertexInputDescription *description) {
    vertexInputInfo.pVertexAttributeDescriptions= description->attributes.data();
    vertexInputInfo.vertexAttributeDescriptionCount= description->attributes.size();

    vertexInputInfo.pVertexBindingDescriptions= description->bindings.data();
    vertexInputInfo.vertexBindingDescriptionCount= description->bindings.size();
}

void VPipeline::Create() {
    shaderStages = vector<VkPipelineShaderStageCreateInfo>({VInitializer::PipelineStageInfo(VK_SHADER_STAGE_VERTEX_BIT, shader->vertModule), VInitializer::PipelineStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, shader->fragModule) });

    viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = colorBlendAttachment.size();
    colorBlending.pAttachments = colorBlendAttachment.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;

    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass->rawRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    VK_CHECK(vkCreateGraphicsPipelines(device->rawDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rawPipeline));
}

void VPipeline::Bind(VCommandBuffer* commandBuffer) const {
    vkCmdBindPipeline(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, rawPipeline);
}

void VPipeline::UpdateConstant(VCommandBuffer* commandBuffer, void* data) const {
    vkCmdPushConstants(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, constantSize, data);
}