#pragma once

#include <mars/graphics/backend/framebuffer.hpp>

#include <vector>
#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_framebuffer {
        std::vector<VkFramebuffer> framebuffers;
    };

    struct vk_framebuffer_impl {
        static framebuffer vk_framebuffer_create(const device& _device, const framebuffer_create_params& _params);
        static void vk_framebuffer_destroy(framebuffer& _framebuffer, const device& _device);
    };
} // namespace mars::graphics::vulkan