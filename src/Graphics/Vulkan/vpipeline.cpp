#include "vpipeline.hpp"
#include "vshader.hpp"
#include "vdevice.hpp"
#include "vrender_pass.hpp"
#include "vinitializer.hpp"
#include "vswapchain.hpp"
#include "vcommand_buffer.hpp"

vpipeline::vpipeline(vdevice* _device) {
    device = _device;

    layout_info = VInitializer::pipeline_layout_info();
    vertex_input_info = VInitializer::pipeline_vertex_input_info();
    input_assembly = VInitializer::pipeline_input_assembly_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    rasterization_info = VInitializer::pipeline_rasterization_info(VK_POLYGON_MODE_FILL);
    multisample_info = VInitializer::pipeline_multisample_info();
    depth_stencil = VInitializer::depth_stencil_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
}

void vpipeline::load_render_pass(vrender_pass* _renderPass) {
    for (auto a : _renderPass->attachments) {
        if (a.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            continue;
        VkPipelineColorBlendAttachmentState color = VInitializer::pipeline_color_blend();
        color_blend_attachment.push_back(color);
    }

    pipeline_info.renderPass = _renderPass->raw_render_pass;
}

void vpipeline::load_shader(VShader* _shader) {
    for (auto m : _shader->modules)
        shader_stages.push_back(VInitializer::pipeline_stage_info(m.flags, m.module));
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &_shader->layout;
}

void vpipeline::create_layout() {
    VK_CHECK(vkCreatePipelineLayout(device->raw_device, &layout_info, nullptr, &pipeline_layout));
}

void vpipeline::apply_input_description(VertexInputDescription *_description) {
    vertex_input_info.pVertexAttributeDescriptions= _description->attributes.data();
    vertex_input_info.vertexAttributeDescriptionCount= _description->attributes.size();

    vertex_input_info.pVertexBindingDescriptions= _description->bindings.data();
    vertex_input_info.vertexBindingDescriptionCount= _description->bindings.size();
}

void vpipeline::apply_viewport(pipeline_viewport _view_data) {
    viewport.x = _view_data.offset.x;
    viewport.y = _view_data.flip_y ? _view_data.size.y : _view_data.offset.y;
    viewport.width = _view_data.size.x;
    viewport.height = _view_data.flip_y ? -_view_data.size.y : _view_data.size.y;
    viewport.minDepth = _view_data.depth.x;
    viewport.maxDepth = _view_data.depth.y;

    scissor.offset = {static_cast<int>(_view_data.offset.x), static_cast<int>(_view_data.offset.y) };
    scissor.extent = {static_cast<uint32_t>(_view_data.size.x), static_cast<uint32_t>(_view_data.size.y) };
}

void vpipeline::create() {

    viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;

    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.attachmentCount = color_blend_attachment.size();
    color_blending.pAttachments = color_blend_attachment.data();
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = nullptr;

    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterization_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.pDepthStencilState = &depth_stencil;

    VK_CHECK(vkCreateGraphicsPipelines(device->raw_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &raw_pipeline));
}

void vpipeline::bind(vcommand_buffer* _command_buffer) const {
    vkCmdBindPipeline(_command_buffer->raw_command_buffers[_command_buffer->record_index], VK_PIPELINE_BIND_POINT_GRAPHICS, raw_pipeline);
}

void vpipeline::update_constant(vcommand_buffer* _command_buffer, void* _data) const {
    vkCmdPushConstants(_command_buffer->raw_command_buffers[_command_buffer->record_index], pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, 0, _data);
}