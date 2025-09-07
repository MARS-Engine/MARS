#include <mars/graphics/backend/vulkan/vk_buffer.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_buffer, 6> buffers;
        log_channel buffer_channel("graphics/vulkan/buffer");

        uint32_t find_memory_type(VkPhysicalDevice _device, uint32_t _type_filter, VkMemoryPropertyFlags _properties) {
            VkPhysicalDeviceMemoryProperties memory_properties;
            vkGetPhysicalDeviceMemoryProperties(_device, &memory_properties);

            for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
                if ((_type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & _properties) == _properties)
                    return i;

            logger::assert_(buffer_channel, "failed to find valid memory type for buffer");
            return -1;
        }

        VkBufferUsageFlagBits mars_buffer_usage_to_vulkan(mars_buffer_type _type) {
            switch (_type) {
            case MARS_BUFFER_TYPE_VERTEX:
                return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            case MARS_BUFFER_TYPE_INDEX:
                return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            }
            return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        }
    } // namespace detail

    buffer vk_buffer_impl::vk_buffer_create(const device& _device, const buffer_create_params& _params) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_buffer* buffer_ptr = detail::buffers.request_entry();

        buffer result;
        result.data = buffer_ptr;
        result.engine = _device.engine;

        VkBufferCreateInfo buffer_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = _params.alocated_size,
            .usage = detail::mars_buffer_usage_to_vulkan(_params.type),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VkResult vk_result = vkCreateBuffer(device_ptr->device, &buffer_info, nullptr, &buffer_ptr->vk_buffer);

        logger::assert_(vk_result == VK_SUCCESS, detail::buffer_channel, "failed to create buffer with error {}", meta::enum_to_string(vk_result));

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device_ptr->device, buffer_ptr->vk_buffer, &memory_requirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memory_requirements.size;
        allocInfo.memoryTypeIndex = detail::find_memory_type(device_ptr->physical_device, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vk_result = vkAllocateMemory(device_ptr->device, &allocInfo, nullptr, &buffer_ptr->device_memory);

        logger::assert_(vk_result == VK_SUCCESS, detail::buffer_channel, "failed to allocate memory with error: {}", meta::enum_to_string(vk_result));

        vk_result = vkBindBufferMemory(device_ptr->device, buffer_ptr->vk_buffer, buffer_ptr->device_memory, 0);

        logger::assert_(vk_result == VK_SUCCESS, detail::buffer_channel, "failed to bind memory with error: {}", meta::enum_to_string(vk_result));

        return result;
    }

    void vk_buffer_impl::vk_buffer_bind(buffer& _buffer, const command_buffer& _command_buffer) {
        vk_buffer* buffer_ptr = _buffer.data.get<vk_buffer>();
        vk_command_pool* command_pool_ptr = _command_buffer.data.get<vk_command_pool>();

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(command_pool_ptr->command_buffers[_command_buffer.buffer_index], 0, 1, &buffer_ptr->vk_buffer, offsets);
    }

    void vk_buffer_impl::vk_buffer_bind_index(buffer& _buffer, const command_buffer& _command_buffer) {
        vk_buffer* buffer_ptr = _buffer.data.get<vk_buffer>();
        vk_command_pool* command_pool_ptr = _command_buffer.data.get<vk_command_pool>();

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindIndexBuffer(command_pool_ptr->command_buffers[_command_buffer.buffer_index], buffer_ptr->vk_buffer, 0, VK_INDEX_TYPE_UINT16);
    }

    void* vk_buffer_impl::vk_buffer_map(buffer& _buffer, const device& _device, size_t _size, size_t _offset) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_buffer* buffer_ptr = _buffer.data.get<vk_buffer>();

        void* result;
        vkMapMemory(device_ptr->device, buffer_ptr->device_memory, _offset, _size, 0, &result);
        return result;
    }

    void vk_buffer_impl::vk_buffer_unmap(buffer& _buffer, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_buffer* buffer_ptr = _buffer.data.get<vk_buffer>();
        vkUnmapMemory(device_ptr->device, buffer_ptr->device_memory);
    }

    void vk_buffer_impl::vk_buffer_destroy(buffer& _buffer, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_buffer* buffer_ptr = _buffer.data.get<vk_buffer>();

        vkDestroyBuffer(device_ptr->device, buffer_ptr->vk_buffer, nullptr);
        vkFreeMemory(device_ptr->device, buffer_ptr->device_memory, nullptr);

        detail::buffers.remove(buffer_ptr);
        _buffer = {};
    }
} // namespace mars::graphics::vulkan