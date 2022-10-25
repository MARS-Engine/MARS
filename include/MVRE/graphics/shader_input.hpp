#ifndef __MVRE__SHADER__INPUT__
#define __MVRE__SHADER__INPUT__

#include <MVRE/resources/resource_manager.hpp>
#include <MVRE/graphics/backend/base/base_shader_input.hpp>

namespace mvre_graphics {

    class shader_input : public mvre_resources::resource_manager {
    private:
        engine_instance* instance = nullptr;
        mvre_graphics_base::base_shader_input* raw_shader_input = nullptr;
    public:
        explicit shader_input(engine_instance* _instance);

        inline void create() { raw_shader_input->create(); }

        inline void bind() { raw_shader_input->bind(); }
        inline void unbind() { raw_shader_input->unbind(); }

        inline mvre_graphics_base::base_buffer* add_buffer(size_t _input_size, mvre_graphics_base::MVRE_MEMORY_TYPE _input_type) { return raw_shader_input->add_buffer(_input_size, _input_type); }
        inline void load_input(mvre_graphics_base::mvre_shader_inputs _inputs) { raw_shader_input->load_input(_inputs); }
    };
}

#endif