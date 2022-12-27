#ifndef MVRE_BACKEND_INSTANCE_
#define MVRE_BACKEND_INSTANCE_

#include <MVRE/debug/debug.hpp>
#include <MVRE/executioner/executioner.hpp>
#include "window.hpp"

namespace mvre_graphics {

    class buffer;
    class shader;
    class shader_input;
    class texture;
    class pipeline;
    class render_pass;
    class command_buffer;
    class shader_data;

    class backend_instance {
    protected:
        mvre_executioner::executioner_job* clear_job = nullptr;
        mvre_executioner::executioner_job* swap_job = nullptr;

        window* raw_window = nullptr;
        bool m_enable_validation = false;
        command_buffer* m_primary_buffer = nullptr;

        virtual buffer* generate_buffer() { return nullptr; }
        virtual shader* generate_shader() { return nullptr; }
        virtual shader_input* generate_shader_input() { return nullptr; }
        virtual texture* generate_texture() { return nullptr; }
        virtual pipeline* generate_pipeline() { return nullptr; }
        virtual render_pass* generate_render_pass() { return nullptr; }
        virtual shader_data* generate_shader_data() { return nullptr; }

        uint32_t m_image_index = 0;
        uint32_t m_current_frame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;

    public:
        inline uint32_t image_index() const { return m_image_index; }
        inline uint32_t current_frame() const { return m_current_frame; }
        inline uint32_t max_frames() const { return MAX_FRAMES_IN_FLIGHT; }

        inline command_buffer* primary_buffer() const { return m_primary_buffer; }
        inline window* get_window() const { return raw_window; }
        inline bool enable_validation_layer() const { return m_enable_validation; }

        explicit backend_instance(bool _enable_validation) { m_enable_validation = _enable_validation; }

        template<typename T> T* instance() { mvre_debug::debug::error((std::string)" T - type - " + typeid(T).name() + " - is not a valid graphic type"); return nullptr; }

        virtual void create_with_window(const std::string& _title, mvre_math::vector2<int> _size) { }

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