#ifndef MARS_COMMAND_BUFFER_BUILDER_
#define MARS_COMMAND_BUFFER_BUILDER_

#include "graphics_builder.hpp"

namespace mars_graphics {
    class command_buffer;

    class command_buffer_builder : graphics_builder<command_buffer> {
    private:
        size_t m_size;
    public:
        command_buffer_builder() = delete;
        explicit command_buffer_builder(const std::shared_ptr<command_buffer>& _ptr);

        command_buffer_builder& set_size(size_t _size) {
            m_size = _size;
            return *this;
        }

        std::shared_ptr<command_buffer> build();
    };
}

#endif