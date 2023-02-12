#ifndef MARS_TEXTURE_
#define MARS_TEXTURE_

#include "graphics_component.hpp"
#include <MARS/resources/resource_manager.hpp>

namespace mars_graphics {

    class texture : public mars_resources::resource_base, public graphics_component {
    protected:
        mars_math::vector2<size_t> m_size {};
        int m_channels = -1;
        unsigned char* m_data = nullptr;
        MARS_FORMAT m_format;
        MARS_TEXTURE_USAGE m_usage;
        MARS_TEXTURE_LAYOUT m_layout =  MARS_TEXTURE_LAYOUT_READONLY;

        /**
         * Load texture to m_data
         * @param _texture_path texture path
         */
        bool load_texture(const std::string& _texture_path);

        /**
         * Free original texture. (Execute after loading into Vulkan, Directx, etc...)
         */
        void free_og_texture();
    public:
        [[nodiscard]] inline MARS_FORMAT format() { return m_format; }
        [[nodiscard]] inline MARS_TEXTURE_LAYOUT layout() { return m_layout; }
        inline void set_size(const mars_math::vector2<size_t>& _size) { m_size = _size; }

        using graphics_component::graphics_component;

        virtual void create(MARS_FORMAT _format, MARS_TEXTURE_USAGE _usage) { }

        virtual void bind() { }
        virtual void unbind() { }
    };
}

#endif