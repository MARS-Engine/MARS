#ifndef __MVRE__V__BACKEND__INSTANCE__
#define __MVRE__V__BACKEND__INSTANCE__

#include <MVRE/graphics/backend/template/backend_instance.hpp>
#include <MVRE/graphics/backend/vulkan/v_window.hpp>
#include <MVRE/graphics/backend/vulkan/v_pipeline.hpp>
#include <MVRE/graphics/backend/vulkan/v_render_pass.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader.hpp>
#include <MVRE/graphics/backend/vulkan/v_command_buffer.hpp>
#include <MVRE/graphics/backend/vulkan/v_texture.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader_input.hpp>
#include <MVRE/graphics/backend/vulkan/v_shader_data.hpp>

namespace mvre_graphics {

    class v_instance;
    class v_device;
    class v_swapchain;
    class v_command_pool;
    class v_framebuffer;
    class v_sync;
    class v_depth;

    class v_backend_instance : public backend_instance {
    private:
        v_instance* m_instance = nullptr;
        v_device* m_device = nullptr;
        v_swapchain* m_swapchain = nullptr;
        v_command_pool* m_command_pool = nullptr;
        v_framebuffer* m_framebuffer = nullptr;
        v_render_pass* m_render_pass = nullptr;
        v_sync* m_sync = nullptr;
        v_depth* m_depth = nullptr;

    protected:
        shader* generate_shader() override { return new v_shader(this); }
        pipeline* generate_pipeline() override { return new v_pipeline(this); }
        render_pass* generate_render_pass() override { return new v_render_pass(this); }
        texture* generate_texture() override { return new v_texture(this); }
        shader_input * generate_shader_input() override { return new v_shader_input(this); }
        shader_data * generate_shader_data() override { return new v_shader_data(this); }
        buffer * generate_buffer() override { return new v_buffer(this); }
    public:
        inline v_render_pass* get_render_pass() const { return m_render_pass; }
        inline v_instance* instance() const { return m_instance; }
        inline v_window* get_vulkan_window() const { return dynamic_cast<v_window*>(get_window()); }
        inline v_device* device() const { return m_device; }
        inline v_swapchain* swapchain() const { return m_swapchain; }
        inline v_command_pool* command_pool() const { return m_command_pool; }
        inline v_framebuffer* framebuffer() const { return m_framebuffer; }
        inline v_sync* sync() const { return m_sync; }
        inline v_depth* depth() const { return m_depth; }

        inline VkCommandBuffer raw_command_buffer() const { return dynamic_cast<v_command_buffer*>(primary_buffer())->raw_command_buffer(); }

        using backend_instance::backend_instance;

        VkCommandBuffer get_single_time_command();
        void end_single_time_command(VkCommandBuffer _command);

        void create_with_window(const std::string& _title, mvre_math::vector2<int> _size) override;

        void update() override;
        void prepare_render() override;
        void draw() override;
        void destroy() override;
    };
}

#endif