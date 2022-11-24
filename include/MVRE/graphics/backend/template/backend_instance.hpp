#ifndef __MVRE__BACKEND__INSTANCE__
#define __MVRE__BACKEND__INSTANCE__

#include <MVRE/debug/debug.hpp>
#include <MVRE/executioner/executioner.hpp>
#include "window.hpp"
#include "command_buffer.hpp"

namespace mvre_graphics {

    class buffer;
    class shader;
    class shader_input;
    class texture;

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

    public:
        inline command_buffer* primary_buffer() { return m_primary_buffer; }
        inline window* get_window() { return raw_window; }

        backend_instance(bool _enable_validation) { m_enable_validation = _enable_validation; }

        template<typename T> T* instance() { mvre_debug::debug::error((std::string)" T - type - " + typeid(T).name() + " - is not a valid graphic type"); }

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
}

#endif