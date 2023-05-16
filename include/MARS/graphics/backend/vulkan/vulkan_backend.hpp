#ifndef MARS_V_BACKEND_INSTANCE_
#define MARS_V_BACKEND_INSTANCE_

#include <MARS/graphics/backend/template/graphics_backend.hpp>
#include <MARS/graphics/backend/vulkan/v_window.hpp>
#include <MARS/graphics/backend/vulkan/v_pipeline.hpp>
#include <MARS/graphics/backend/vulkan/v_render_pass.hpp>
#include <MARS/graphics/backend/vulkan/v_shader.hpp>
#include <MARS/graphics/backend/vulkan/v_command_buffer.hpp>
#include <MARS/graphics/backend/vulkan/v_texture.hpp>
#include <MARS/graphics/backend/vulkan/v_shader_input.hpp>
#include <MARS/graphics/backend/vulkan/v_shader_data.hpp>
#include <MARS/graphics/backend/vulkan/v_framebuffer.hpp>
#include <MARS/graphics/backend/vulkan/v_backend/v_device.hpp>
#include <MARS/graphics/backend/vulkan/v_swapchain.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

namespace mars_graphics {

    class v_instance;
    class v_swapchain;
    class v_command_pool;
    class v_sync;

    class vulkan_backend : public graphics_backend {
    private:
        v_instance* m_vulkan_instance = nullptr;
        std::shared_ptr<device> m_device = nullptr;
        v_command_pool* m_command_pool = nullptr;
        v_sync* m_sync = nullptr;

    protected:
        texture_builder texture_build() override { return texture_builder{ std::make_shared<v_texture>(shared_from_this()) }; }
        buffer_builder buffer_build() override { return buffer_builder{ std::make_shared<v_buffer>(shared_from_this()) }; }
        framebuffer_builder framebuffer_build() override { return framebuffer_builder{ std::make_shared<v_framebuffer>(shared_from_this()) }; }
        shader_data_builder shader_data_build() override { return shader_data_builder{ std::make_shared<v_shader_data>(shared_from_this()) }; }
        render_pass_builder render_pass_build() override { return render_pass_builder{ std::make_shared<v_render_pass>(shared_from_this()) }; }
        shader_input_builder shader_input_build() override { return shader_input_builder{ std::make_shared<v_shader_input>(shared_from_this()) }; }
        pipeline_builder pipeline_build() override { return pipeline_builder{ std::make_shared<v_pipeline>(shared_from_this()) }; }
        shader_builder shader_build() override { return shader_builder{ std::make_shared<v_shader>(shared_from_this()) }; }
        device_builder device_build() override { return device_builder{ std::make_shared<v_device>(shared_from_this()) }; }
    public:
        [[nodiscard]] inline v_instance* instance() const { return m_vulkan_instance; }
        [[nodiscard]] inline v_window* get_vulkan_window() const { return raw_window->cast<v_window>(); }
        [[nodiscard]] inline v_device* get_device() const { return m_device->cast<v_device>(); }
        [[nodiscard]] inline v_swapchain* swapchain() const { return dynamic_cast<v_swapchain*>(m_swapchain); }
        [[nodiscard]] inline v_command_pool* command_pool() const { return m_command_pool; }
        [[nodiscard]] inline renderer* instance_renderer() const { return dynamic_cast<renderer*>(m_renderer); }
        [[nodiscard]] inline v_sync* sync() const { return m_sync; }

        [[nodiscard]] inline VkCommandBuffer raw_command_buffer() const { return dynamic_cast<v_command_buffer*>(primary_buffer())->raw_command_buffer(); }

        using graphics_backend::graphics_backend;

        VkCommandBuffer get_single_time_command();
        void end_single_time_command(VkCommandBuffer _command);

        void create_with_window(const std::string& _title, const mars_math::vector2<int>& _size, const std::string& _renderer) override;

        void update() override;
        void prepare_render() override;
        void draw() override;
        void destroy() override;
        void wait_idle() override;
    };
}

#endif