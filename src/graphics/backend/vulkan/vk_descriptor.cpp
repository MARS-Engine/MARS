#include "mars/debug/logger.hpp"
#include "mars/graphics/backend/descriptor.hpp"
#include "mars/graphics/backend/vulkan/vk_command_pool.hpp"
#include <array>
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_descriptor.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/graphics/backend/vulkan/vk_buffer.hpp>
#include <mars/graphics/backend/vulkan/vk_buffer_view.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_pipeline.hpp>
#include <mars/graphics/backend/vulkan/vk_texture.hpp>
#include <mars/meta.hpp>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_descriptor, 6> descriptors;
        sparse_vector<vk_descriptor_set, 6> descriptor_sets;
        log_channel descriptor_channel("graphics/vulkan/descriptor");
    } // namespace detail

    descriptor vk_descriptor_impl::vk_descriptor_create(const device& _device, const descriptor_create_params& _params, size_t _frames_in_flight) {
        descriptor result;
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_descriptor* descriptor_ptr = detail::descriptors.request_entry();

        result.engine = _device.engine;
        result.data = descriptor_ptr;
        result.frames_in_flight = _frames_in_flight;

        std::array<VkDescriptorPoolSize, _params.descriptors_size.size()> pool_sizes;

        size_t used_size = 0;

        for (size_t i = 0; i < pool_sizes.size(); i++) {
            switch (static_cast<mars_descriptor_type>(i)) {
            case MARS_DESCRIPTOR_TYPE_SAMPLER:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_SAMPLER;
                break;
            case MARS_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            case MARS_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;
            case MARS_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            case MARS_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                break;
            case MARS_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                break;
            case MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case MARS_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                break;
            case MARS_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                break;
            case MARS_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                pool_sizes[used_size].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                break;
            }

            pool_sizes[used_size].descriptorCount = static_cast<uint32_t>(_params.descriptors_size[i]);

            if (pool_sizes[used_size].descriptorCount != 0)
                used_size++;
        }

        VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = static_cast<uint32_t>(_params.max_sets * _frames_in_flight),
            .poolSizeCount = static_cast<uint32_t>(used_size),
            .pPoolSizes = pool_sizes.data(),
        };

        VkResult vk_result = vkCreateDescriptorPool(device_ptr->device, &pool_info, nullptr, &descriptor_ptr->descriptor_pool);

        logger::assert_(vk_result == VK_SUCCESS, detail::descriptor_channel, "failed to create descriptor pool {}", meta::enum_to_string(vk_result));

        return result;
    }

    descriptor_set vk_descriptor_impl::vk_descriptor_set_create(const descriptor& _descriptor, const device& _device, const pipeline& _pipeline, const std::vector<descriptor_set_create_params>& _params) {
        descriptor_set result;

        vk_descriptor* descriptor_ptr = _descriptor.data.get<vk_descriptor>();
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_pipeline* piepline_ptr = _pipeline.data.get<vk_pipeline>();
        vk_descriptor_set* descriptor_set_ptr = detail::descriptor_sets.request_entry();

        result.engine = _descriptor.engine;
        result.data = descriptor_set_ptr;

        std::vector<VkDescriptorSetLayout> layouts(_descriptor.frames_in_flight, piepline_ptr->descriptor_set_layout);

        VkDescriptorSetAllocateInfo alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptor_ptr->descriptor_pool,
            .descriptorSetCount = static_cast<uint32_t>(_descriptor.frames_in_flight),
            .pSetLayouts = layouts.data(),
        };

        descriptor_set_ptr->descriptor_sets.resize(_descriptor.frames_in_flight);

        VkResult vk_result = vkAllocateDescriptorSets(device_ptr->device, &alloc_info, descriptor_set_ptr->descriptor_sets.data());

        logger::assert_(vk_result == VK_SUCCESS, detail::descriptor_channel, "failed to allocate descriptor sets with error code {}", meta::enum_to_string(vk_result));

        for (size_t i = 0; i < _descriptor.frames_in_flight; i++) {

            std::vector<VkDescriptorBufferInfo> buffer_infos;
            std::vector<VkDescriptorImageInfo> texture_infos;

            buffer_infos.reserve(_params[i].buffers.size() + _params[i].buffer_views.size());
            texture_infos.reserve(_params[i].textures.size());

            std::vector<VkWriteDescriptorSet> descriptor_sets;

            for (const std::pair<buffer, size_t>& entry : _params[i].buffers) {
                vk_buffer* entry_ptr = entry.first.data.get<vk_buffer>();
                VkDescriptorBufferInfo* info = &buffer_infos.emplace_back(VkDescriptorBufferInfo({
                    .buffer = entry_ptr->vk_buffer,
                    .offset = 0,
                    .range = entry.first.allocated_size,
                }));

                VkWriteDescriptorSet descriptor_write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptor_set_ptr->descriptor_sets[i],
                    .dstBinding = static_cast<uint32_t>(entry.second),
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = info,
                };
                descriptor_sets.push_back(descriptor_write);
            }

            for (const std::pair<texture, size_t>& entry : _params[i].textures) {
                vk_texture* entry_ptr = entry.first.data.get<vk_texture>();
                VkDescriptorImageInfo* info = &texture_infos.emplace_back(VkDescriptorImageInfo({
                    .sampler = entry_ptr->sampler,
                    .imageView = entry_ptr->view,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                }));

                VkWriteDescriptorSet descriptor_write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptor_set_ptr->descriptor_sets[i],
                    .dstBinding = static_cast<uint32_t>(entry.second),
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = info,
                };

                descriptor_sets.push_back(descriptor_write);
            }

            for (const std::pair<buffer_view, size_t>& entry : _params[i].buffer_views) {
                vk_buffer_view* entry_ptr = entry.first.data.get<vk_buffer_view>();
                vk_buffer* buffer_ptr = entry.first.src_buffer.data.get<vk_buffer>();
                VkDescriptorBufferInfo* info = &buffer_infos.emplace_back(VkDescriptorBufferInfo({
                    .buffer = buffer_ptr->vk_buffer,
                    .offset = 0,
                    .range = entry.first.src_buffer.allocated_size,
                }));

                VkWriteDescriptorSet descriptor_write{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptor_set_ptr->descriptor_sets[i],
                    .dstBinding = static_cast<uint32_t>(entry.second),
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
                    .pTexelBufferView = &entry_ptr->vk_buffer_view,
                };

                descriptor_sets.push_back(descriptor_write);
            }

            vkUpdateDescriptorSets(device_ptr->device, static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data(), 0, nullptr);
        }

        return result;
    }

    void vk_descriptor_impl::vk_descriptor_set_bind(const descriptor_set& _descriptor_set, const command_buffer& _command_buffer, const pipeline& _pipeline, size_t _current_frame) {
        vk_descriptor_set* descriptor_set_ptr = _descriptor_set.data.get<vk_descriptor_set>();
        vk_pipeline* pipeline_ptr = _pipeline.data.get<vk_pipeline>();
        vk_command_pool* command_ptr = _command_buffer.data.get<vk_command_pool>();

        vkCmdBindDescriptorSets(command_ptr->command_buffers[_command_buffer.buffer_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ptr->pipeline_layout, 0, 1, &descriptor_set_ptr->descriptor_sets[_current_frame], 0, nullptr);
    }

    void vk_descriptor_impl::vk_descriptor_destroy(descriptor& _descriptor, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_descriptor* descriptor_ptr = _descriptor.data.get<vk_descriptor>();
        vkDestroyDescriptorPool(device_ptr->device, descriptor_ptr->descriptor_pool, nullptr);

        detail::descriptors.remove(descriptor_ptr);
        _descriptor = {};
    }
} // namespace mars::graphics::vulkan