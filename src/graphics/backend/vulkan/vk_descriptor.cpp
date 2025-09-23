#include "mars/debug/logger.hpp"
#include "mars/graphics/backend/vulkan/vk_command_pool.hpp"
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_descriptor.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/graphics/backend/vulkan/vk_buffer.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_pipeline.hpp>
#include <mars/meta.hpp>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_descriptor, 6> descriptors;
        sparse_vector<vk_descriptor_set, 6> descriptor_sets;
        log_channel descriptor_channel("graphics/vulkan/descriptor");
    } // namespace detail

    descriptor vk_descriptor_impl::vk_descriptor_create(const device& _device, size_t _frames_in_flight) {
        descriptor result;
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_descriptor* descriptor_ptr = detail::descriptors.request_entry();

        result.engine = _device.engine;
        result.data = descriptor_ptr;
        result.frames_in_flight = _frames_in_flight;

        VkDescriptorPoolSize pool_size{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(_frames_in_flight),
        };

        VkDescriptorPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(_frames_in_flight),
            .poolSizeCount = 1,
            .pPoolSizes = &pool_size,
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
            std::vector<VkWriteDescriptorSet> descriptor_sets;

            for (const buffer& entry : _params[i].buffers) {
                vk_buffer* entry_ptr = entry.data.get<vk_buffer>();
                VkDescriptorBufferInfo* aaaa = &buffer_infos.emplace_back(VkDescriptorBufferInfo({
                    .buffer = entry_ptr->vk_buffer,
                    .offset = 0,
                    .range = entry.allocated_size,
                }));

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = descriptor_set_ptr->descriptor_sets[i];
                descriptorWrite.dstBinding = 0;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = aaaa;
                descriptor_sets.push_back(descriptorWrite);
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