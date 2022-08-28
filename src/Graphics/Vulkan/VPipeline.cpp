#include "VPipeline.hpp"
#include "VShader.hpp"
#include "VDevice.hpp"
#include "VRenderPass.hpp"
#include "VInitializer.hpp"
#include "VSwapchain.hpp"
#include "VCommandBuffer.hpp"

VPipeline::VPipeline(VDevice* _device) {
    device = _device;

    layoutInfo = VInitializer::PipelineLayoutInfo();
    vertexInputInfo = VInitializer::PipelineVertexInputInfo();
    inputAssembly = VInitializer::PipelineInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    rasterizationInfo = VInitializer::PipelineRasterizationInfo(VK_POLYGON_MODE_FILL);
    multisampleInfo = VInitializer::PipelineMultisampleInfo();
    depthStencil = VInitializer::DepthStencilInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
}

void VPipeline::LoadRenderPass(VRenderPass* renderPass) {
    for (auto a : renderPass->attachments) {
        if (a.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            continue;
        VkPipelineColorBlendAttachmentState color = VInitializer::PipelineColorBlend();
        colorBlendAttachment.push_back(color);
    }

    pipelineInfo.renderPass = renderPass->rawRenderPass;
}

void VPipeline::LoadShader(VShader* shader) {
    for (auto m : shader->modules)
        shaderStages.push_back(VInitializer::PipelineStageInfo(m.flags, m.module));
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &shader->layout;
}

void VPipeline::CreateLayout() {
    VK_CHECK(vkCreatePipelineLayout(device->rawDevice, &layoutInfo, nullptr, &pipelineLayout));
}

void VPipeline::ApplyInputDescription(VertexInputDescription *description) {
    vertexInputInfo.pVertexAttributeDescriptions= description->attributes.data();
    vertexInputInfo.vertexAttributeDescriptionCount= description->attributes.size();

    vertexInputInfo.pVertexBindingDescriptions= description->bindings.data();
    vertexInputInfo.vertexBindingDescriptionCount= description->bindings.size();
}

void VPipeline::ApplyViewport(PipelineViewport viewData) {
    viewport.x = viewData.offset.x;
    viewport.y = viewData.flipY ? viewData.size.y : viewData.offset.y;
    viewport.width = viewData.size.x;
    viewport.height = viewData.flipY ? -viewData.size.y : viewData.size.y;
    viewport.minDepth = viewData.depth.x;
    viewport.maxDepth = viewData.depth.y;

    scissor.offset = {static_cast<int>(viewData.offset.x), static_cast<int>(viewData.offset.y) };
    scissor.extent = {static_cast<uint32_t>(viewData.size.x), static_cast<uint32_t>(viewData.size.y) };
}

void VPipeline::Create() {

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
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    VK_CHECK(vkCreateGraphicsPipelines(device->rawDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &rawPipeline));
}

void VPipeline::Bind(VCommandBuffer* commandBuffer) const {
    vkCmdBindPipeline(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, rawPipeline);
}

void VPipeline::UpdateConstant(VCommandBuffer* commandBuffer, void* data) const {
    vkCmdPushConstants(commandBuffer->rawCommandBuffers[commandBuffer->recordIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 0, data);
}