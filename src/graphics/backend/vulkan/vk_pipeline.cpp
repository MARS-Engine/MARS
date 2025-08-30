#include <mars/graphics/backend/vulkan/vk_pipeline.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/shader.hpp>
#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_render_pass.hpp>
#include <mars/graphics/backend/vulkan/vk_shader.hpp>
#include <mars/meta.hpp>

#include <array>
#include <cstdint>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_pipeline, 16> pipelines;
        log_channel pipeline_channel("graphics/vulkan/pipeline");
    } // namespace detail

    pipeline vk_pipeline_impl::vk_pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup) {
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_render_pass* render_pass_ptr = static_cast<vk_render_pass*>(_render_pass.data);
        vk_pipeline* pipeline_ptr = detail::pipelines.request_entry();

        pipeline result;
        result.data = pipeline_ptr;
        result.engine = _device.engine;

        vk_shader* shader_ptr = static_cast<vk_shader*>(_setup.pipeline_shader.data);

        // at most there are 6 stages (tesselation counts as two seperate stages)
        std::array<VkPipelineShaderStageCreateInfo, 6> shader_stages;

        for (int i = 0; i < shader_ptr->modules.size(); i++) {
            shader_stages[i] = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .module = shader_ptr->modules[i].module,
                .pName = "main",
            };

            switch (shader_ptr->modules[i].type) {
            case MARS_SHADER_TYPE_VERTEX:
                shader_stages[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case MARS_SHADER_TYPE_FRAGMENT:
                shader_stages[i].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            }
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .vertexAttributeDescriptionCount = 0,
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data(),
        };

        VkPipelineViewportStateCreateInfo viewport_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo color_blending{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
        };

        VkPipelineLayoutCreateInfo pipeline_layout_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };

        VkResult vk_result = vkCreatePipelineLayout(device_ptr->device, &pipeline_layout_info, nullptr, &pipeline_ptr->pipeline_layout);

        logger::assert_(vk_result == VK_SUCCESS, detail::pipeline_channel, "failed to create pipeline layout with error code {}", meta::enum_to_string(vk_result));

        VkGraphicsPipelineCreateInfo pipeline_info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32_t>(shader_ptr->modules.size()),
            .pStages = shader_stages.data(),
            .pVertexInputState = &vertex_input_info,
            .pInputAssemblyState = &input_assembly,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &color_blending,
            .pDynamicState = &dynamic_state,
            .layout = pipeline_ptr->pipeline_layout,
            .renderPass = render_pass_ptr->vk_render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
        };

        vk_result = vkCreateGraphicsPipelines(device_ptr->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_ptr->vk_pipeline);

        logger::assert_(vk_result == VK_SUCCESS, detail::pipeline_channel, "failed to create pipeline with error code {}", meta::enum_to_string(vk_result));

        return result;
    }

    void vk_pipeline_impl::vk_pipeline_bind(const pipeline& _pipeline, const command_buffer& _command_buffer, const pipeline_bind_params& _params) {
        vk_pipeline* pipeline_ptr = static_cast<vk_pipeline*>(_pipeline.data);
        vk_command_pool* command_pool_ptr = static_cast<vk_command_pool*>(_command_buffer.data);

        vkCmdBindPipeline(command_pool_ptr->command_buffers[_command_buffer.buffer_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ptr->vk_pipeline);

        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(_params.size.x),
            .height = static_cast<float>(_params.size.y),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(command_pool_ptr->command_buffers[_command_buffer.buffer_index], 0, 1, &viewport);

        VkRect2D scissor{
            .offset = { 0, 0 },
            .extent = { static_cast<uint32_t>(_params.size.x), static_cast<uint32_t>(_params.size.y) },
        };

        vkCmdSetScissor(command_pool_ptr->command_buffers[_command_buffer.buffer_index], 0, 1, &scissor);
    }

    void vk_pipeline_impl::vk_pipeline_destroy(pipeline& _pipeline, const device& _device) {
        vk_device* device_ptr = static_cast<vk_device*>(_device.data);
        vk_pipeline* pipeline_ptr = static_cast<vk_pipeline*>(_pipeline.data);
        vkDestroyPipeline(device_ptr->device, pipeline_ptr->vk_pipeline, nullptr);
        vkDestroyPipelineLayout(device_ptr->device, pipeline_ptr->pipeline_layout, nullptr);
    }
} // namespace mars::graphics::vulkan