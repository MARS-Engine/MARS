#include <mars/graphics/imgui/imgui.hpp>

#include <mars/graphics/backend/vulkan/vk_command_pool.hpp>
#include <mars/graphics/backend/vulkan/vk_descriptor.hpp>
#include <mars/graphics/backend/vulkan/vk_device.hpp>
#include <mars/graphics/backend/vulkan/vk_instance.hpp>
#include <mars/graphics/backend/vulkan/vk_render_pass.hpp>
#include <mars/graphics/backend/vulkan/vulkan_backend.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

namespace mars {
    void imgui_initialize(imgui_initialize_params _params) {
        if (_params.g_instance.engine != graphics::vulkan_t::get_functions())
            return;

        ImGui_ImplSDL3_InitForVulkan(_params.g_window.sdl_window);

        graphics::vulkan::vk_instance* instance_data = _params.g_instance.data.get<graphics::vulkan::vk_instance>();
        graphics::vulkan::vk_device* device_data = _params.g_device.data.get<graphics::vulkan::vk_device>();
        graphics::vulkan::vk_descriptor* descriptor_data = _params.g_descriptor.data.get<graphics::vulkan::vk_descriptor>();
        graphics::vulkan::vk_render_pass* render_pass_data = _params.g_render_pass.data.get<graphics::vulkan::vk_render_pass>();

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance_data->instance;
        init_info.PhysicalDevice = device_data->physical_device;
        init_info.Device = device_data->device;
        init_info.QueueFamily = device_data->queue_indices.graphics_family;
        init_info.Queue = device_data->graphics_queue;
        init_info.DescriptorPool = descriptor_data->descriptor_pool;
        init_info.RenderPass = render_pass_data->vk_render_pass;
        init_info.MinImageCount = _params.min_image;
        init_info.ImageCount = _params.image_count;
        ImGui_ImplVulkan_Init(&init_info);
    }

    void imgui_render(const command_buffer& _command_buffer) {
        if (_command_buffer.engine != graphics::vulkan_t::get_functions())
            return;

        graphics::vulkan::vk_command_pool* pool = _command_buffer.data.get<graphics::vulkan::vk_command_pool>();
        ImDrawData* dd = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(dd, pool->command_buffers[_command_buffer.buffer_index]);
    }
} // namespace mars