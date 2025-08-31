#include "mars/graphics/backend/command_pool.hpp"
#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/meta.hpp>

#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_command_pool, 6> command_pools;
        log_channel command_pool_channel("graphics/vulkan/command_pool");
    } // namespace detail

    command_pool vk_command_pool_impl::vk_command_pool_create(const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_command_pool* command_pool_ptr = detail::command_pools.request_entry();

        command_pool result;
        result.engine = _device.engine;
        result.data = command_pool_ptr;

        VkCommandPoolCreateInfo pool_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = device_ptr->queue_indices.graphics_family,
        };

        VkResult vk_result = vkCreateCommandPool(device_ptr->device, &pool_info, nullptr, &command_pool_ptr->command_pool);

        logger::assert_(vk_result == VK_SUCCESS, detail::command_pool_channel, "failed to create command pool with error {}", meta::enum_to_string(vk_result));

        return result;
    }

    std::vector<command_buffer> vk_command_pool_impl::vk_command_buffer_create(const command_pool& _command_pool, const device& _device, size_t _n_command_buffers) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_command_pool* command_pool_ptr = _command_pool.data.get<vk_command_pool>();

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = command_pool_ptr->command_pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = _n_command_buffers;

        size_t size = command_pool_ptr->command_buffers.size();

        command_pool_ptr->command_buffers.resize(size + _n_command_buffers);

        VkResult vk_result = vkAllocateCommandBuffers(device_ptr->device, &allocInfo, command_pool_ptr->command_buffers.data() + size);

        logger::assert_(vk_result == VK_SUCCESS, detail::command_pool_channel, "failed to create command buffer", meta::enum_to_string(vk_result));

        std::vector<command_buffer> result;
        result.resize(_n_command_buffers);

        for (size_t i = 0; i < result.size(); i++) {
            result[i].engine = _command_pool.engine;
            result[i].data = _command_pool.data;
            result[i].buffer_index = i;
        }

        return result;
    }

    void vk_command_pool_impl::vk_command_buffer_reset(const command_buffer& _command_buffer) {
        vk_command_pool* command_pool_ptr = _command_buffer.data.get<vk_command_pool>();
        vkResetCommandBuffer(command_pool_ptr->command_buffers[_command_buffer.buffer_index], 0);
    }

    void vk_command_pool_impl::vk_command_buffer_record(const command_buffer& _command_buffer) {
        vk_command_pool* command_pool_ptr = _command_buffer.data.get<vk_command_pool>();

        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };

        VkResult result = vkBeginCommandBuffer(command_pool_ptr->command_buffers[_command_buffer.buffer_index], &begin_info);

        logger::assert_(result == VK_SUCCESS, detail::command_pool_channel, "failed to being record command buffer {} with error {}", _command_buffer.buffer_index, meta::enum_to_string(result));
    }

    void vk_command_pool_impl::vk_command_buffer_record_end(const command_buffer& _command_buffer) {
        vk_command_pool* command_pool_ptr = _command_buffer.data.get<vk_command_pool>();

        VkResult result = vkEndCommandBuffer(command_pool_ptr->command_buffers[_command_buffer.buffer_index]);

        logger::assert_(result == VK_SUCCESS, detail::command_pool_channel, "failed to end record command buffer {} with error {}", _command_buffer.buffer_index, meta::enum_to_string(result));
    }

    void vk_command_pool_impl::vk_command_buffer_draw(const command_buffer& _command_buffer, const command_buffer_draw_params& _params) {
        vk_command_pool* command_pool_ptr = _command_buffer.data.get<vk_command_pool>();

        vkCmdDraw(command_pool_ptr->command_buffers[_command_buffer.buffer_index], _params.vertex_count, _params.instance_count, _params.first_vertex, _params.first_instance);
    }

    void vk_command_pool_impl::vk_command_pool_destroy(command_pool& _command_pool, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_command_pool* command_pool_ptr = _command_pool.data.get<vk_command_pool>();

        vkDestroyCommandPool(device_ptr->device, command_pool_ptr->command_pool, nullptr);

        _command_pool = {};
    }
} // namespace mars::graphics::vulkan