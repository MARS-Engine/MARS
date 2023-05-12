#ifndef MARS_BACKEND_INSTANCE_
#define MARS_BACKEND_INSTANCE_

#include <MARS/memory/mars_ref.hpp>
#include <MARS/debug/debug.hpp>
#include "window.hpp"
#include <pl/safe_deque.hpp>

#include "builders/texture_builder.hpp"
#include "builders/buffer_builder.hpp"
#include "builders/framebuffer_builder.hpp"
#include "builders/shader_data_builder.hpp"
#include "builders/render_pass_builder.hpp"
#include "builders/shader_input_builder.hpp"
#include "builders/pipeline_builder.hpp"
#include "builders/shader_builder.hpp"

namespace mars_resources {
    class resource_manager;
}

namespace mars_graphics {

    //graphics types
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
    class graphics_engine;

    class graphics_backend : public std::enable_shared_from_this<graphics_backend> {
    protected:
        mars_ref<mars_resources::resource_manager> m_resources;

        window* raw_window = nullptr;
        bool m_enable_validation = false;
        command_buffer* m_primary_buffer = nullptr;
        mars_ref<graphics_engine> m_graphics;
        swapchain* m_swapchain = nullptr;
        renderer* m_renderer = nullptr;

        virtual texture_builder texture_build() { return texture_builder{ nullptr }; }
        virtual buffer_builder buffer_build() { return buffer_builder{ nullptr }; }
        virtual framebuffer_builder framebuffer_build() { return framebuffer_builder{ nullptr }; }
        virtual shader_data_builder shader_data_build() { return shader_data_builder{ nullptr }; }
        virtual render_pass_builder render_pass_build() { return render_pass_builder{ nullptr }; }
        virtual shader_input_builder shader_input_build() { return shader_input_builder{ nullptr }; }
        virtual pipeline_builder pipeline_build() { return pipeline_builder{ nullptr }; }
        virtual shader_builder shader_build() { return shader_builder{ nullptr }; }

        uint32_t m_index = 0;
        uint32_t m_current_frame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;

        light_manager* m_light = nullptr;
    public:
        [[nodiscard]] inline mars_ref<mars_resources::resource_manager> resources() const { return m_resources; }
        inline void set_resources(const mars_ref<mars_resources::resource_manager>& _resource_manager) { m_resources = _resource_manager; }

        [[nodiscard]] inline uint32_t index() const { return m_index; }
        [[nodiscard]] inline uint32_t current_frame() const { return m_current_frame; }
        [[nodiscard]] inline uint32_t max_frames() const { return MAX_FRAMES_IN_FLIGHT; }

        [[nodiscard]] inline command_buffer* primary_buffer() const { return m_primary_buffer; }
        [[nodiscard]] inline window* get_window() const { return raw_window; }
        [[nodiscard]] inline bool enable_validation_layer() const { return m_enable_validation; }
        [[nodiscard]] inline swapchain* get_swapchain() const { return m_swapchain; }
        [[nodiscard]] inline renderer* get_renderer() const { return m_renderer; }

        inline void set_graphics(const mars_ref<graphics_engine>& _graphics) { m_graphics = _graphics; }

        [[nodiscard]] inline light_manager* lights() const { return m_light; }

        [[nodiscard]] std::string render_type() const;

        explicit graphics_backend(bool _enable_validation) { m_enable_validation = _enable_validation; }

        template<typename T> mars_ref<T> create() { mars_debug::debug::error((std::string)" T - type - " + typeid(T).name() + " - is not a valid graphic type"); }
        template<typename T> T builder() { mars_debug::debug::error((std::string)" T - type - " + typeid(T).name() + " - is not a valid graphic builder type"); }

        virtual void create_with_window(const std::string& _title, const mars_math::vector2<size_t>& _size, const std::string& _renderer) { }

        virtual void update() { }
        virtual void prepare_render() { }
        virtual void draw() { }
        virtual void destroy() { }
        virtual void wait_idle() { }
    };

    template<> inline texture_builder graphics_backend::builder<texture_builder>() { return texture_build(); }
    template<> inline buffer_builder graphics_backend::builder<buffer_builder>() { return buffer_build(); }
    template<> inline framebuffer_builder graphics_backend::builder<framebuffer_builder>() { return framebuffer_build(); }
    template<> inline shader_data_builder graphics_backend::builder<shader_data_builder>() { return shader_data_build(); }
    template<> inline render_pass_builder graphics_backend::builder<render_pass_builder>() { return render_pass_build(); }
    template<> inline shader_input_builder graphics_backend::builder<shader_input_builder>() { return shader_input_build(); }
    template<> inline pipeline_builder graphics_backend::builder<pipeline_builder>() { return pipeline_build(); }
    template<> inline shader_builder graphics_backend::builder<shader_builder>() { return shader_build(); }
}

#endif