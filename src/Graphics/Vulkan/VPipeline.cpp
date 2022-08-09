#include "VPipeline.hpp"
#include "VShader.hpp"
#include "VDevice.hpp"
#include "VRenderPass.hpp"
#include "VInitializer.hpp"
#include "VSwapchain.hpp"
#include "VCommandBuffer.hpp"

VPipeline::VPipeline(VDevice* _device, VSwapchain* _swapchain, VRenderPass* _renderPass) {
    device = _device;
    renderPass = _renderPass;
    swapchain = _swapchain;

    layoutInfo = VInitializer::PipelineLayoutInfo();
    vertexInputInfo = VInitializer::PipelineVertexInputInfo();
    inputAssembly = VInitializer::PipelineInputAssemblyInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    rasterizationInfo = VInitializer::PipelineRasterizationInfo(VK_POLYGON_MODE_FILL);
    multisampleInfo = VInitializer::PipelineMultisampleInfo();
    colorBlendAttachment = VInitializer::PipelineColorBlend();
    depthStencil = VInitializer::DepthStencilInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
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
    VK_CHECK(vkCreatePipelineLayout(device->rawDevice, &layoutInfo, nullptr, &pipelineLayout));
}

void VPipeline::Create(VShader* _shader, VertexInputDescription* description) {
    shader = _shader;
    shaderStages = vector<VkPipelineShaderStageCreateInfo>({VInitializer::PipelineStageInfo(VK_SHADER_STAGE_VERTEX_BIT, shader->vertModule), VInitializer::PipelineStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, shader->fragModule) });

    if (description != nullptr) {
        vertexInputInfo.pVertexAttributeDescriptions= description->attributes.data();
        vertexInputInfo.vertexAttributeDescriptionCount= description->attributes.size();

        vertexInputInfo.pVertexBindingDescriptions= description->bindings.data();
        vertexInputInfo.vertexBindingDescriptionCount= description->bindings.size();
    }

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = swapchain->size.x;
    viewport.height = swapchain->size.y;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    scissor.offset = { 0, 0 };
    scissor.extent = { (uint32_t)swapchain->size.x, (uint32_t)swapchain->size.y };

    viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

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