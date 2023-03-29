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
#include <MARS/graphics/backend/vulkan/v_swapchain.hpp>
#include <MARS/graphics/renderer/renderer.hpp>

namespace mars_graphics {

    class v_instance;
    class v_device;
    class v_swapchain;
    class v_command_pool;
    class v_sync;

    class vulkan_backend : public graphics_backend {
    private:
        v_instance* m_vulkan_instance = nullptr;
        v_device* m_device = nullptr;
        v_command_pool* m_command_pool = nullptr;
        v_sync* m_sync = nullptr;

    protected:
        std::shared_ptr<shader> generate_shader() override { return std::make_shared<v_shader>(this); }
        std::shared_ptr<pipeline> generate_pipeline() override { return std::make_shared<v_pipeline>(this); }
        std::shared_ptr<render_pass> generate_render_pass() override { return std::make_shared<v_render_pass>(this); }
        std::shared_ptr<texture> generate_texture() override { return std::make_shared<v_texture>(this); }
        std::shared_ptr<shader_input> generate_shader_input() override { return std::make_shared<v_shader_input>(this); }
        std::shared_ptr<shader_data> generate_shader_data() override { return std::make_shared<v_shader_data>(this); }
        std::shared_ptr<buffer> generate_buffer() override { return std::make_shared<v_buffer>(this); }
        std::shared_ptr<framebuffer> generate_framebuffer() override { return std::make_shared<v_framebuffer>(this); }
    public:
        [[nodiscard]] inline v_instance* instance() const { return m_vulkan_instance; }
        [[nodiscard]] inline v_window* get_vulkan_window() const { return dynamic_cast<v_window*>(get_window()); }
        [[nodiscard]] inline v_device* device() const { return m_device; }
        [[nodiscard]] inline v_swapchain* swapchain() const { return dynamic_cast<v_swapchain*>(m_swapchain); }
        [[nodiscard]] inline v_command_pool* command_pool() const { return m_command_pool; }
        [[nodiscard]] inline renderer* instance_renderer() const { return dynamic_cast<renderer*>(m_renderer); }
        [[nodiscard]] inline v_sync* sync() const { return m_sync; }

        [[nodiscard]] inline VkCommandBuffer raw_command_buffer() const { return dynamic_cast<v_command_buffer*>(primary_buffer())->raw_command_buffer(); }

        using graphics_backend::graphics_backend;

        VkCommandBuffer get_single_time_command();
        void end_single_time_command(VkCommandBuffer _command);

        void create_with_window(const std::string& _title, const mars_math::vector2<size_t>& _size, const std::string& _renderer) override;

        void update() override;
        void prepare_render() override;
        void draw() override;
        void destroy() override;
        void wait_idle() override;
    };
}

#endif