#include "mars/meta.hpp"
#include <cstdint>
#include <mars/graphics/backend/vulkan/vk_buffer_view.hpp>

#include <mars/container/sparse_array.hpp>
#include <mars/debug/logger.hpp>
#include <mars/graphics/backend/vulkan/vk_buffer.hpp>
#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_utils.hpp>
#include <vulkan/vulkan_core.h>

namespace mars::graphics::vulkan {
    namespace detail {
        sparse_vector<vk_buffer_view, 24> buffer_views;
        log_channel buffer_view_channel("graphics/vulkan/buffer_view");
    } // namespace detail

    buffer_view vk_buffer_view_impl::vk_buffer_view_create(const device& _device, const buffer_view_create_params& _params) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_buffer* buffer_ptr = _params.src_buffer.data.get<vk_buffer>();
        vk_buffer_view* buffer_view_ptr = detail::buffer_views.request_entry();

        buffer_view result;
        result.data = buffer_view_ptr;
        result.src_buffer = _params.src_buffer;

        VkBufferViewCreateInfo view_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .buffer = buffer_ptr->vk_buffer,
            .format = mars_to_vk(_params.format),
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        VkResult vk_result = vkCreateBufferView(device_ptr->device, &view_info, nullptr, &buffer_view_ptr->vk_buffer_view);

        logger::assert_(vk_result == VK_SUCCESS, detail::buffer_view_channel, "failed to create pipeline descriptor set layout with error {}", meta::enum_to_string(vk_result));

        return result;
    }

    void vk_buffer_view_impl::vk_buffer_view_destroy(buffer_view& _buffer_view, const device& _device) {
        vk_device* device_ptr = _device.data.get<vk_device>();
        vk_buffer_view* buffer_view_ptr = _buffer_view.data.get<vk_buffer_view>();

        detail::buffer_views.remove(buffer_view_ptr);
        _buffer_view = {};
    }
} // namespace mars::graphics::vulkan