#ifndef MARS_BUFFER_BUILDER_
#define MARS_BUFFER_BUILDER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "../graphics_builder.hpp"

namespace mars_graphics {
    class buffer;

    struct buffer_data {
        size_t size = 0;
        size_t frames = 1;
        MARS_MEMORY_TYPE mem_type;
    };

    class buffer_builder : graphics_builder<buffer> {
    private:
        buffer_data m_data;
    public:
        buffer_builder() = delete;

        using graphics_builder::graphics_builder;

        inline buffer_builder& set_size(size_t _size) {
            m_data.size = _size;
            return *this;
        }

        inline buffer_builder& set_type(MARS_MEMORY_TYPE _type) {
            m_data.mem_type = _type;
            return *this;
        }

        inline buffer_builder& set_frames(size_t _frames) {
            m_data.frames = _frames;
            return *this;
        }

        std::shared_ptr<buffer> build();
    };
}

#endif