#ifndef MARS_TEXTURE_SRC_
#define MARS_TEXTURE_SRC_

#include "single_time_command.hpp"
#include <MARS/math/vector4.hpp>

namespace mars_graphics {
    class texture;
    class buffer;

    class texture_stc : single_time_command {
    private:
        std::shared_ptr<texture> m_texture;
    public:
        texture_stc(const std::shared_ptr<graphics_backend>& _backend, const std::shared_ptr<texture>& _texture) : single_time_command(_backend) {
            m_texture = _texture;
        }

        void begin_buffer_copy();
        void finish_buffer_copy();
        void copy_buffer_to_image(const std::shared_ptr<buffer>& _buffer, const mars_math::vector4<uint32_t>& _rect);

        ~texture_stc() {
            base_execute();
        }
    };
}

#endif