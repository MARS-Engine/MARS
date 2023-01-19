#ifndef MARS_BACKEND_INSTANCE_
#define MARS_BACKEND_INSTANCE_

#include <MARS/debug/debug.hpp>
#include <MARS/executioner/executioner.hpp>
#include "window.hpp"

namespace mars_graphics {

    class buffer;
    class shader;
    class shader_input;
    class texture;
    class pipeline;
    class render_pass;
    class command_buffer;
    class shader_data;
    class graphics_instance;
    class light_manager;

    class backend_instance {
    protected:
        mars_executioner::executioner_job* clear_job = nullptr;
        mars_executioner::executioner_job* swap_job = nullptr;

        window* raw_window = nullptr;
        bool m_enable_validation = false;
        command_buffer* m_primary_buffer = nullptr;
        graphics_instance* m_instance = nullptr;

        virtual buffer* generate_buffer() { return nullptr; }
        virtual shader* generate_shader() { return nullptr; }
        virtual shader_input* generate_shader_input() { return nullptr; }
        virtual texture* generate_texture() { return nullptr; }
        virtual pipeline* generate_pipeline() { return nullptr; }
        virtual render_pass* generate_render_pass() { return nullptr; }
        virtual shader_data* generate_shader_data() { return nullptr; }

        uint32_t m_current_frame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;

        light_manager* m_light;

        bool m_is_deferred_renderer_enabled = false;
    public:
        [[nodiscard]] inline uint32_t current_frame() const { return m_current_frame; }
        [[nodiscard]] inline uint32_t max_frames() const { return MAX_FRAMES_IN_FLIGHT; }

        [[nodiscard]] inline command_buffer* primary_buffer() const { return m_primary_buffer; }
        [[nodiscard]] inline window* get_window() const { return raw_window; }
        [[nodiscard]] inline bool enable_validation_layer() const { return m_enable_validation; }

        inline void set_instance(graphics_instance* _instance) { m_instance = _instance; }

        inline void set_deferred_renderer(bool _enabled) { m_is_deferred_renderer_enabled = _enabled; }

        [[nodiscard]] inline light_manager* lights() const { return m_light; }

        [[nodiscard]] inline std::string render_type() const {
            return m_is_deferred_renderer_enabled ? "deferred" : "forward";
        }

        explicit backend_instance(bool _enable_validation) { m_enable_validation = _enable_validation; }

        template<typename T> T* instance() { mars_debug::debug::error((std::string)" T - type - " + typeid(T).name() + " - is not a valid graphic type"); }

        virtual void create_with_window(const std::string& _title, const mars_math::vector2<int>& _size) { }

        virtual void update() { }
        virtual void prepare_render() { }
        virtual void draw() { }
        virtual void destroy() { }
    };

    /* template specialization */
    template<> inline buffer* backend_instance::instance<buffer>() { return generate_buffer(); }
    template<> inline shader* backend_instance::instance<shader>() { return generate_shader(); }
    template<> inline shader_input* backend_instance::instance<shader_input>() { return generate_shader_input(); }
    template<> inline texture* backend_instance::instance<texture>() { return generate_texture(); }
    template<> inline pipeline* backend_instance::instance<pipeline>() { return generate_pipeline(); }
    template<> inline render_pass* backend_instance::instance<render_pass>() { return generate_render_pass(); }
    template<> inline shader_data* backend_instance::instance<shader_data>() { return generate_shader_data(); }
}

#endif