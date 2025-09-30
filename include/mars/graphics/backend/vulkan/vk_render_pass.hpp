#pragma once

#include <mars/graphics/backend/command_pool.hpp>
#include <mars/graphics/backend/framebuffer.hpp>
#include <mars/graphics/backend/render_pass.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_render_pass {
        VkRenderPass vk_render_pass;
    };

    struct vk_render_pass_impl {
        static render_pass vk_render_pass_create(const device& _device, const render_pass_create_params& _params);
        static void vk_render_pass_bind(const render_pass& _render_pass, const command_buffer& _command_buffer, const framebuffer& _framebuffer, const render_pass_bind_param& _params);
        static void vk_render_pass_unbind(const render_pass& _render_pass, const command_buffer& _command_buffer);
        static void vk_render_pass_destroy(render_pass& _render_pass, const device& _device);
    };
} // namespace mars::graphics::vulkan