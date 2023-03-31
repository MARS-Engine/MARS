#ifndef MARS_BACKEND_INSTANCE_
#define MARS_BACKEND_INSTANCE_

#include <MARS/debug/debug.hpp>
#include "window.hpp"

namespace mars_resources {
    class _resource_manager;
    typedef std::shared_ptr<_resource_manager> resource_manager;
}

namespace mars_graphics {

    class buffer;
    class shader;
    class shader_input;
    class texture;
    class pipeline;
    class render_pass;
    class command_buffer;
    class shader_data;
    class light_manager;
    class renderer;
    class swapchain;
    class framebuffer;

    class _graphics_engine;
    typedef std::shared_ptr<_graphics_engine> graphics_engine;

    class graphics_backend {
    protected:
        mars_resources::resource_manager m_resources;

        window* raw_window = nullptr;
        bool m_enable_validation = false;
        command_buffer* m_primary_buffer = nullptr;
        graphics_engine m_graphics;
        swapchain* m_swapchain = nullptr;
        renderer* m_renderer = nullptr;

        virtual std::shared_ptr<buffer> generate_buffer() { return nullptr; }
        virtual std::shared_ptr<shader> generate_shader() { return nullptr; }
        virtual std::shared_ptr<shader_input> generate_shader_input() { return nullptr; }
        virtual std::shared_ptr<texture> generate_texture() { return nullptr; }
        virtual std::shared_ptr<pipeline> generate_pipeline() { return nullptr; }
        virtual std::shared_ptr<render_pass> generate_render_pass() { return nullptr; }
        virtual std::shared_ptr<shader_data> generate_shader_data() { return nullptr; }
        virtual std::shared_ptr<framebuffer> generate_framebuffer() { return nullptr; }

        uint32_t m_index = 0;
        uint32_t m_current_frame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;

        light_manager* m_light = nullptr;
    public:
        [[nodiscard]] inline mars_resources::resource_manager resources() const { return m_resources; }
        inline void set_resources(const mars_resources::resource_manager& _resource_manager) { m_resources = _resource_manager; }

        [[nodiscard]] inline uint32_t index() const { return m_index; }
        [[nodiscard]] inline uint32_t current_frame() const { return m_current_frame; }
        [[nodiscard]] inline uint32_t max_frames() const { return MAX_FRAMES_IN_FLIGHT; }

        [[nodiscard]] inline command_buffer* primary_buffer() const { return m_primary_buffer; }
        [[nodiscard]] inline window* get_window() const { return raw_window; }
        [[nodiscard]] inline bool enable_validation_layer() const { return m_enable_validation; }
        [[nodiscard]] inline swapchain* get_swapchain() const { return m_swapchain; }
        [[nodiscard]] inline renderer* get_renderer() const { return m_renderer; }

        inline void set_graphics(const graphics_engine& _graphics) { m_graphics = _graphics; }

        [[nodiscard]] inline light_manager* lights() const { return m_light; }

        [[nodiscard]] std::string render_type() const;

        explicit graphics_backend(bool _enable_validation) { m_enable_validation = _enable_validation; }

        template<typename T> std::shared_ptr<T> create() { mars_debug::debug::error((std::string)" T - type - " + typeid(T).name() + " - is not a valid graphic type"); }

        virtual void create_with_window(const std::string& _title, const mars_math::vector2<size_t>& _size, const std::string& _renderer) { }

        virtual void update() { }
        virtual void prepare_render() { }
        virtual void draw() { }
        virtual void destroy() { }
        virtual void wait_idle() { }
    };

    /* template specialization */
    template<> inline std::shared_ptr<buffer> graphics_backend::create<buffer>() { return generate_buffer(); }
    template<> inline std::shared_ptr<shader> graphics_backend::create<shader>() { return generate_shader(); }
    template<> inline std::shared_ptr<shader_input> graphics_backend::create<shader_input>() { return generate_shader_input(); }
    template<> inline std::shared_ptr<texture> graphics_backend::create<texture>() { return generate_texture(); }
    template<> inline std::shared_ptr<pipeline> graphics_backend::create<pipeline>() { return generate_pipeline(); }
    template<> inline std::shared_ptr<render_pass> graphics_backend::create<render_pass>() { return generate_render_pass(); }
    template<> inline std::shared_ptr<shader_data> graphics_backend::create<shader_data>() { return generate_shader_data(); }
    template<> inline std::shared_ptr<framebuffer> graphics_backend::create<framebuffer>() { return generate_framebuffer(); }
}

#endif