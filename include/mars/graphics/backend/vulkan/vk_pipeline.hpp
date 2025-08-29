#pragma once

#include <mars/graphics/backend/pipeline.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_pipeline {
        VkPipelineLayout pipeline_layout;
        VkPipeline vk_pipeline;
    };

    struct vk_pipeline_impl {
        static pipeline vk_pipeline_create(const device& _device, const render_pass& _render_pass, const pipeline_setup& _setup);
        static void vk_pipeline_destroy(pipeline& _pipeline, const device& _device);
    };
} // namespace mars::graphics::vulkan