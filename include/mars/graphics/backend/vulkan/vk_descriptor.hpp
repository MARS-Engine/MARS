#pragma once

#include <mars/graphics/backend/descriptor.hpp>
#include <mars/graphics/backend/device.hpp>

#include <vulkan/vulkan.h>

namespace mars::graphics::vulkan {

    struct vk_descriptor {
        VkDescriptorPool descriptor_pool;
    };

    struct vk_descriptor_set {
        std::vector<VkDescriptorSet> descriptor_sets;
    };

    struct vk_descriptor_impl {
        static descriptor vk_descriptor_create(const device& _device, size_t _frames_in_flight);
        static descriptor_set vk_descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params);
        static void vk_descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame);
        static void vk_descriptor_destroy(descriptor& _descriptor, const device& _device);
    };
} // namespace mars::graphics::vulkan