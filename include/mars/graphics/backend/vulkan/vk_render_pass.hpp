#pragma once

#include <mars/graphics/backend/render_pass.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_render_pass {
        VkRenderPass vk_render_pass;
    };

    struct vk_render_pass_impl {
        static render_pass vk_render_pass_create(const device& _device, const swapchain& _swapchain);
        static void vk_render_pass_destroy(render_pass& _render_pass, const device& _device);
    };
} // namespace mars::graphics::vulkan