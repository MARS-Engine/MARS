#pragma once

#include <mars/graphics/backend/shader.hpp>
#include <mars/graphics/functional/device.hpp>

#include <vector>
#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_shader_module {
        mars_shader_type type;
        VkShaderModule module;
    };

    struct vk_shader {
        std::vector<vk_shader_module> modules;
    };

    struct vk_shader_impl {
        static shader vk_shader_create(device& _device, const std::vector<shader_module>& _shaders);
        static void vk_shader_destroy(shader& _shader, const device& _device);
    };
} // namespace mars::graphics::vulkan