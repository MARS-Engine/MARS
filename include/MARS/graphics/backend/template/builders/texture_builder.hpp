#ifndef MARS_TEXTURE_BUILDER_
#define MARS_TEXTURE_BUILDER_

#include <MARS/graphics/backend/template/graphics_types.hpp>
#include "../graphics_builder.hpp"

namespace mars_graphics {
    class texture;
    class buffer;

    struct texture_data {
        mars_math::vector2<size_t> size;
        MARS_FORMAT format;
        MARS_TEXTURE_USAGE usage;
        MARS_TEXTURE_LAYOUT layout = MARS_TEXTURE_LAYOUT_READONLY;
    };

    class texture_builder : graphics_builder<texture> {
    private:
        bool m_already_initialize = false;
        bool m_complete = false;
        texture_data m_data;
    public:
        texture_builder() = delete;

        using graphics_builder::graphics_builder;

        texture_builder& set_size(const mars_math::vector2<size_t>& _size) {
            m_data.size = _size;
            return *this;
        }

        texture_builder& set_format(MARS_FORMAT _format) {
            m_data.format = _format;
            return *this;
        }

        texture_builder& set_usage(MARS_TEXTURE_USAGE _usage) {
            m_data.usage = _usage;
            return *this;
        }

        texture_builder& set_layout(MARS_TEXTURE_LAYOUT _layout) {
            m_data.layout = _layout;
            return *this;
        }

        texture_builder& initialize();
        texture_builder& copy_buffer_to_image(buffer* _buffer, const mars_math::vector4<uint32_t>& _rect);
        texture_builder& load_from_file(const std::string& _path);

        std::shared_ptr<texture> build();
    };
}

#endif